; $Id: lxapihlp.asm,v 1.11 2004/07/19 22:37:14 smilcke Exp $

;
; lxapi.asm
; Autor:               Stefan Milcke
; Erstellt am:         06.04.2003
; Letzte Aenderung am: 18.05.2004
;
       .386p

       include seg32.inc
       include devhlp.inc
       include r0thunk.inc
       include lxbases.inc
       include lxbasem.inc

DATA16 segment
ddname         db[10]  dup (0)
ddtable        db[12] dup (0)
IdcPtr         dd 0
DATA16 ends

;*******************************************************************************
;* Get TKSSBase                                                               *
;*******************************************************************************
DosTable2 struc
       d2_ErrMap24                     dd ?
       d2_MsgMap24                     dd ?
       d2_Err_Table_24                 dd ?
       d2_CDSAddr                      dd ?
       d2_GDT_RDR1                     dd ?
       d2_InterruptLevel               dd ?
       d2__cInDos                      dd ?
       d2_zero_1                       dd ?
       d2_zero_2                       dd ?
       d2_FlatCS                       dd ?
       d2_FlatDS                       dd ?
       d2__TKSSBase                    dd ?
       d2_intSwitchStack               dd ?
       d2_privateStack                 dd ?
       d2_PhysDiskTablePtr             dd ?
       d2_forceEMHandler               dd ?
       d2_ReserveVM                    dd ?
       d2_pgpPageDir                   dd ?
       d2_unknown                      dd ?
DosTable2 ends

DATA32 segment
       public DevHelp32
       public _TKSSBase
       public intSwitchStack
DevHelp32      dd 0
_TKSSBase      dd 0
intSwitchStack dd 0
DATA32 ends

TEXT32 segment
       ASSUME CS:FLAT, DS:FLAT, ES:FLAT
       public GetTKSSBase
       extrn  DevHlp : near
       ALIGN   4
GetTKSSBase proc near
       push    ebp
       mov     ebp, esp
       push    es
       push    ebx
       push    ecx
       push    edx

       ;
       ; Gets the TKSSBase pointer from DosTable. TKSSBase is used by
       ; __StackToFlat() to convert a stack based address to a FLAT address
       ; without the overhead of DevHlp_VirtToLin
       ;
       ; DosTable is obtained through GetDOSVar with undocumented index 9
       ; The layout is :
       ;     byte      count of following dword (n)
       ;     dword 1   -+
       ;       .        |
       ;       .        | this is DosTable1
       ;       .        |
       ;     dword n   -+
       ;     byte      count of following dword (p)
       ;     dword 1   -+
       ;       .        |
       ;       .        | this is DosTable2
       ;       .        |
       ;     dword p   -+
       ;
       ; Flat CS  is dword number 10 in DosTable2
       ; Flat DS  is dword number 11 in DosTable2
       ; TKSSBase is dword number 12 in DosTable2
       ;
       mov 	eax, 9                       ; undocumented DosVar : DosTable pointer
       xor 	ecx, ecx
       mov 	edx, DevHlp_GetDOSVar
       call 	DevHlp
       jc      short GetTKSSBase_Err
       mov 	es, ax                       ; es:bx points to DosTable
       movzx 	ebx, bx
       movzx 	ecx, byte ptr es:[ebx]     ; count of dword in DosTable1
       mov 	eax, es:[ebx + 4 * ecx + 2].d2__TKSSBase
       mov 	_TKSSBase, eax

       mov 	eax, es:[ebx + 4 * ecx + 2].d2_intSwitchStack
       mov     intSwitchStack, eax

       xor 	eax, eax
GetTKSSBase_Err:
       pop     edx
       pop     ecx
       pop     ebx
       pop     es
       leave
       ret
GetTKSSBase endp
TEXT32 ends

;*******************************************************************************
;* init_lxapi                                                                  *
;*******************************************************************************
DATA16 segment
       public lxapi32_name
       public lxapi32_attach_err
lxapi32_name db "LXAPI32$",0
lxapi32_attach_err db "Unable to attach to LXAPI32.SYS",0dh,0ah,0
DATA16 ends

DATA32 segment
       public  LX_THUNKAPI
       public  LX_v_required
       public  LX_v_actual
       public  LX_FixSelDPLPtr
       public  LX_RestoreSelDPLPtr
       public  LX_StackAllocPtr
       public  LX_StackFreePtr
       public  LX_IrqStackAllocPtr
       public  LX_IrqStackFreePtr
       public  tempeax
       public  tempedx
       public  tempesi
       public  cpuflags
       public  fInitStack
LX_THUNKAPI            label byte
LX_v_required          dd 0
LX_v_actual            dd 0
LX_FixSelDPLPtr        dd 0
LX_RestoreSelDPLPtr    dd 0
LX_StackAllocPtr       dd 0
LX_StackFreePtr        dd 0
LX_IrqStackAllocPtr    dd 0
LX_IrqStackFreePtr     dd 0
tempeax    dd 0
tempedx    dd 0
tempesi    dd 0
cpuflags   dd 0
fInitStack dd 0
DATA32 ends

CODE16 segment
       assume cs:CODE16, ds:DATA16
       ALIGN   2
       public  thunk3216_init_lxapi_asm
thunk3216_init_lxapi_asm proc far
       push    ds
       push    es
       push    fs
       push    ax
       push    bx
       push    cx
       push    dx
       push    di
       push    si

       push    DATA16
       pop     ds

       pushad
       mov     bx,offset lxapi32_name
       mov     di,offset ddtable
       mov     dl,DevHlp_AttachDD
       call    dword ptr ds:DevHelpInit
       popad
       jc      short @F
       mov     eax,dword ptr (ddtable+6)
       mov     dword ptr IdcPtr,eax
       call    dword ptr ds:IdcPtr
@@:
       pop     si
       pop     di
       pop     dx
       pop     cx
       pop     bx
       pop     ax
       pop     fs
       pop     es
       pop     ds
       jmp     far ptr FLAT:thunk1632_init_lxapi_asm
thunk3216_init_lxapi_asm endp
CODE16 ends

TEXT32 segment
       ASSUME CS:FLAT, DS:FLAT, ES:FLAT
       public  init_lxapi_asm32
init_lxapi_asm32 proc far
       cmp     dword ptr LX_FixSelDPLPtr,0
       jne     short init_lxapi_asm32_ok
       pushad
       pushfd
       mov     ebx,offset FLAT:LX_THUNKAPI
       mov     edx,ebx
       shr     edx,16
       xor     ecx,ecx
       jmp     far ptr CODE16:thunk3216_init_lxapi_asm
       ALIGN   4
thunk1632_init_lxapi_asm:
       cmp     dword ptr LX_FixSelDPLPtr,0
       je      short @F
       call    GetTKSSBase
@@:    popfd
       popad
init_lxapi_asm32_ok:
       retf
init_lxapi_asm32 endp
TEXT32 ends

;*******************************************************************************
;* Attach                                                                      *
;*******************************************************************************
CODE16 segment
       assume cs:CODE16, ds:DATA16
       public thunk3216_attachdd
       extrn   DevHelpInit : dword
       ALIGN   2
thunk3216_attachdd:
       push    ds
       push    DATA16
       pop     ds
       mov     bx,offset ddname
       mov     di,offset ddtable
       mov     dl,DevHlp_AttachDD
       call    dword ptr ds:DevHelpInit
       setc    dl
       xor     eax,eax
       mov     al,dl
       pop     ds
       jmp     far ptr FLAT:thunk1632_attachdd

       public thunk3216_callidc
       ALIGN   2
thunk3216_callidc:
       ; eax -> fptr
       ; ecx -> cmd
       ; ebx -> value
       push    ds
       push    DATA16
       pop     ds
       mov     edx,ebx             ; Value to edx
       shr     edx,16              ; Shift upper 16 bits of ULONG parameter
       mov     dword ptr IdcPtr,eax
       call    dword ptr ds:IdcPtr ; Call IDC entry point
       pop     ds
       jmp     far ptr FLAT:thunk1632_callidc

       ALIGN   2
CODE16 ends

TEXT32 segment
       ASSUME CS:FLAT, DS:FLAT, ES:FLAT
       public _AttachToDDASM
       public _CallIDC
       ALIGN   4
_AttachToDDASM proc near
       push    ebp
       mov     ebp,esp
       push    ebx
       push    ecx
       push    edx
       push    esi
       push    edi
       ThunkStackTo16_Int
       jmp     far ptr CODE16:thunk3216_attachdd

       ALIGN   4
thunk1632_attachdd:
       ThunkStackTo32_Int
       pop     edi
       pop     esi
       pop     edx
       pop     ecx
       pop     ebx
       pop     ebp
       ret
_AttachToDDASM endp

       ALIGN   4
_CallIDC proc near
       push    ebp
       mov     ebp,esp
       push    ebx
       push    ecx
       push    edx
       push    esi
       push    edi
       ; 08h[ebp] ->    table
       ; 0ch[ebp] ->    cmd
       ; 10h[ebp] ->    value
       mov     eax,08h[ebp]
       mov     eax,[eax+6]
       mov     ecx,0ch[ebp]
       mov     ebx,10h[ebp]
       ThunkStackTo16_Int
       jmp     far ptr CODE16:thunk3216_callidc
thunk1632_callidc:
       ThunkStackTo32_Int
       pop     edi
       pop     esi
       pop     edx
       pop     ecx
       pop     ebx
       pop     ebp
       ret
_CallIDC endp

TEXT32 ends

;*******************************************************************************
;* helpers                                                                     *
;*******************************************************************************
DATA32 segment
       public  _ddtable32
       public  _ddname32
_ddtable32 dw  OFFSET DATA16:ddtable
           dw  SEG DATA16:ddtable
_ddname32  dw  OFFSET DATA16:ddname
           dw  SEG DATA16:ddname
DATA32 ends

END

