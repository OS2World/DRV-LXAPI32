; $Id: lxapistrt.asm,v 1.12 2006/02/16 23:07:19 smilcke Exp $

;
; statup.asm
; Autor:               Stefan Milcke
; Erstellt am:         08.12.2001
; Letzte Aenderung am: 22.02.2004
;
       .386p


       INCL_DOS        equ 1
       INCL_DOSERRORS  equ 1
       include os2.inc
       include seg32.inc
       include r0thunk.inc
       include rp.inc
       include devhlp.inc
       include lxbases.inc
       include lxbasem.inc
       NODEVHELPINITVAR    equ 1
       include lxapihlp.inc

MAX_GDTSELECTORS       EQU 64
MAX_MESSAGESIZE        EQU 2048

; Status word masks
STERR      EQU 8000H       ; Bit 15 - Error
STINTER    EQU 0400H       ; Bit 10 - Interim character
STBUI      EQU 0200H       ; Bit  9 - Busy
STDON      EQU 0100H       ; Bit  8 - Done
STECODE    EQU 00FFH       ; Error code

;##############################################################################
;* DATA16
;##############################################################################
DATA16 segment
       public DevHelpInit
       public fOpen
       public  _MESSAGE_STR
       public  _MSG_TABLE16
       extrn  __OffFinalDS16 : byte
       extrn  DriverFileName : byte
DevHelpInit    dd 0
fOpen      dd 0
InitPktSeg dw 0
InitPktOff dw 0
_MESSAGE_STR   db MAX_MESSAGESIZE dup (0)
_MSG_TABLE16   dw 0
               dw OFFSET _MESSAGE_STR    ; message far pointer
               dw SEG    _MESSAGE_STR

;needed for rmcalls.lib
_RM_Help0      dd 0
_RM_Help1      dd 0
_RM_Help3      dd 0
_RMFlags       dd 0
DATA16 ends

;##############################################################################
;* CODE16
;##############################################################################
CODE16 segment
       assume cs:CODE16, ds:DATA16
       extrn DOSOPEN       : far
       extrn DOSWRITE      : far
       extrn DOSCLOSE      : far
       extrn  __OffFinalCS16 : byte

;******************************************************************************
;* Strategy routines                                                          *
;******************************************************************************
;------------------------------ help_stup_strategy -----------------------------
       public  help_stub_strategy
help_stub_strategy proc far
       enter   0,0
       and     sp,0fffch   ; align stack

       mov     ax, DATA16
       mov     ds, ax

       movzx   eax, byte ptr es:[bx].reqCommand
       cmp     eax, 0              ; Init
       je      short @@help_init
       cmp     eax, 0Dh            ; DosOpen
       jne     short @@help_error
;DosOpen
       cmp     word ptr fOpen,0
       je      short @@help_ret_ok     ; not ours

       push    bx             ; later we need this
       push    es

       mov     word ptr fOpen,0
       mov     ax, word ptr InitPktSeg
       mov     fs, ax              ; fs:ebx = req. packet
       xor     ebx, ebx
       mov     bx, word ptr InitPktOff
       call    far ptr FLAT:STRATEGY_

       pop     es
       pop     bx             ; restore ebx ptr
@@help_ret:
       mov     word ptr es:[bx].reqStatus,ax
@@help_ret_error:
       leave
       ret
@@help_init:
       mov     eax, dword ptr es:[bx].i_devHelp
       mov     dword ptr DevHelpInit, eax
       mov     word ptr es:[bx].o_codeend,offset CODE16:__OffFinalCS16
       mov     word ptr es:[bx].o_dataend,offset DATA16:__OffFinalDS16
@@help_ret_ok:
       mov     ax, STDON
       jmp     short @@help_ret
@@help_error:
       mov     ax, STDON + STERR + ERROR_I24_BAD_COMMAND
       mov     word ptr es:[bx].reqStatus, ax
       jmp     short @@help_ret_error
help_stub_strategy endp

;----------------------------- device_stub_strategy ----------------------------
       public  device_stub_strategy
       ALIGN   2
device_stub_strategy proc far
       enter   0,0
       and     sp,0fffch   ; align stack

       mov     ax, DATA16
       mov     ds, ax

       movzx   eax, byte ptr es:[bx].reqCommand
       cmp     eax, 0              ; Init
       jz      short @@init

       push    bx
       push    es
       mov     ax, bx
       xor     ebx, ebx
       mov     bx, ax
       mov     ax, es
       mov     fs, ax              ; fs:ebx = req. packet
       call    far ptr FLAT:STRATEGY_          ; 32 bits strategy entry point

       pop     es
       pop     bx             ; restore bx ptr
       mov     word ptr es:[bx].reqStatus, ax  ; status code

@@device_ret:
       leave
       ret

@@init:
        ;
        ; DEVICE= initialization
        ;
       mov     word ptr InitPktSeg, es
       mov     word ptr InitPktOff, bx
       inc     word ptr fOpen

       call    device_init

       mov     word ptr es:[bx].reqStatus, ax  ; status code (ret by device_init)
       mov     word ptr es:[bx].o_codeend,offset CODE16:__OffFinalCS16
       mov     word ptr es:[bx].o_dataend,offset DATA16:__OffFinalDS16
       jmp     short @@device_ret

init_err:
       mov     dword ptr es:[bx].i_devHelp, 0
       jmp     short @@device_ret

device_stub_strategy endp

;*******************************************************************************
;* DevHelp thunking                                                            *
;*******************************************************************************
       public  thunk3216_devhelp
       ALIGN   2
thunk3216_devhelp:
       push    ds
       push    DATA16
       pop     ds
       call    dword ptr ds:DevHelpInit
       pop     ds
       jmp     far ptr FLAT:thunk1632_devhelp

       public  thunk3216_devhelp_modified_ds
       ALIGN   2
thunk3216_devhelp_modified_ds:
       push    gs
       push    DATA16
       pop     gs
       call    dword ptr gs:DevHelpInit
       pop     gs
       jmp     far ptr FLAT:thunk1632_devhelp_modified_ds

;--------------------------------- device_init ---------------------------------
       public  device_init
       ALIGN   2
device_init proc near
       enter   24, 0
       push    ds
       push    es
       push    bx
       push    si
       push    di

       ; bp      ->  old bp
       ; bp - 2  -> FileHandle
       ; bp - 4  -> ActionTaken
       ; bp - 8  -> IOCTL parm (4 bytes)
       ; bp - 24 -> IOCTL data (16 bytes)

       ; Open BT32HLP$
       push    seg DATA16                 ; seg  FileName
       push    offset DriverFileName
       push    ss                         ; seg &FileHandle
       lea     ax, [bp - 2]
       push    ax                         ; ofs &FileHandle
       push    ss                         ; seg &ActionTaken
       lea     ax, [bp - 4]
       push    ax                         ; ofs &ActionTaken
       push    dword ptr 0                ; file size
       push    0                          ; file attributes
       push    OPEN_ACTION_FAIL_IF_NEW + OPEN_ACTION_OPEN_IF_EXISTS
       push    OPEN_SHARE_DENYNONE + OPEN_ACCESS_READONLY
       push    dword ptr 0                ; reserved
       call    DOSOPEN
       cmp     ax, NO_ERROR
       jnz     short @@error


       ;
       ; Closes BT32HLP$
       ;
       push    word ptr [bp - 2]                   ; FileHandle
       call    DOSCLOSE
       cmp     ax, NO_ERROR
       jnz     short @@error

@@out:
       push    eax

       push    0001H
       push    DATA16
       push    offset _MESSAGE_STR
       push    word ptr _MSG_TABLE16
       push    ss
       lea     dx,[bp - 2]
       push    dx
       call    far ptr DOSWRITE

       pop     eax

       pop     di
       pop     si
       pop     bx
       pop     es
       pop     ds
       leave
       ret
@@error:
       mov     ax, STDON + STERR + ERROR_I24_GEN_FAILURE
       jmp     short @@out

device_init endp

CODE16 ends

;##############################################################################
;* TEXT32
;##############################################################################
TEXT32 segment
       ASSUME CS:FLAT, DS:FLAT, ES:FLAT
       public __SEGSTRT_TEXT32
       extrn  DOSIODELAYCNT : ABS

__SEGSTRT_TEXT32:

;******************************************************************************
;* strategy routine                                                           *
;******************************************************************************
       ALIGN   4
       public STRATEGY_
       extrn  STRATEGY_ENTRY : NEAR
       extrn StratIOCtl_ : NEAR
STRATEGY_ proc far
       mov     eax,DOS32FLATDS
       mov     ds,eax
       mov     es,eax
       cmp     dword ptr LX_FixSelDPLPtr,0
       jne     @F
       call    far ptr FLAT:init_lxapi_asm32
       cmp     dword ptr LX_FixSelDPLPtr,0
       je      @STRATEGY_ERROR

@@:    ThunkStackTo32
       cmp     eax,0
       jne     @STRATEGY_ERROR
       cmp     byte ptr fs:[ebx].reqCommand,010h
       je      @STRATEGY_IOCTL

       call    STRATEGY_ENTRY
       ThunkStackTo16
@STRATEGY_ERROR:
       retf

@STRATEGY_IOCTL:
       call    StratIOCtl_
       ThunkStackTo16
       retf
STRATEGY_ endp

;*******************************************************************************
;* DevHelp thunking                                                            *
;*******************************************************************************
;------------------------------------ DevHlp -----------------------------------
       public  DevHlp
       ALIGN   4
DevHlp proc    near
       ThunkStackTo16_Int
       jmp     far ptr CODE16:thunk3216_devhelp
thunk1632_devhelp:
       ThunkStackTo32_Int
       ret
DevHlp endp

;------------------------------ DevHlp_ModifiedDS ------------------------------
       public  DevHlp_ModifiedDS
       ALIGN   4
DevHlp_ModifiedDS proc near
       ThunkStackTo16_Int
       jmp far ptr CODE16:thunk3216_devhelp_modified_ds
       ALIGN   4
thunk1632_devhelp_modified_ds:
       ThunkStackTo32_Int
       ret
DevHlp_ModifiedDS endp

;*******************************************************************************
;* misc helpers                                                                *
;*******************************************************************************
;---------------------------------- iodelay32_ ---------------------------------
       public  iodelay32_
       ALIGN   4
iodelay32_ proc near
       mov   eax, DOSIODELAYCNT
@@:    dec   eax
       jnz   @b
       loop  iodelay32_
       ret
iodelay32_ endp

TEXT32 ends

;##############################################################################
;* DATA32
;##############################################################################
DATA32 segment
       public  __OffsetFinalCS16
       public  __OffsetFinalDS16
       public  __OffBeginDS32
       public stackbase
       public stacksel
       public  _MSG_TABLE32

__OffBeginDS32         dd 0
stacksel               dd 0
stackbase              dd 0

__OffsetFinalCS16  dw OFFSET CODE16:__OffFinalCS16
__OffsetFinalDS16  dw OFFSET DATA16:__OffFinalDS16
_MSG_TABLE32       dw OFFSET  DATA16:_MSG_TABLE16
                   dw SEG DATA16:_MSG_TABLE16

DATA32 ends

end

