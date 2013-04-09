; $Id: lxbasem.inc,v 1.20 2006/04/27 21:17:53 smilcke Exp $

;
; lxbasem.inc
; Autor:               Stefan Milcke
; Erstellt am:         09.02.2003
; Letzte Aenderung am: 26.04.2006
;

; Defines an IRQ handler for irq (nr)
IRQHandler MACRO nr
CODE16 SEGMENT
       ASSUME  cs:CODE16, ds:DATA16
       ALIGN   2
IRQHandler16_&nr PROC FAR
       mov     ebx,nr
       jmp     IRQHandler16
IRQHandler16_&nr ENDP
CODE16 ENDS
DATA32 SEGMENT
       PUBLIC  _IRQHandler16_&nr&_PTR
_IRQHandler16_&nr&_PTR dw  OFFSET CODE16:IRQHandler16_&nr
                       dw  SEG CODE16:IRQHandler16_&nr
DATA32 ENDS
ENDM

SEGMARKER MACRO segname,markername
segname SEGMENT
	ASSUME CS:FLAT, DS:FLAT, ES:FLAT
	PUBLIC markername
markername:
segname ENDS
ENDM

SEGBEGINMARKER MACRO segname
segname SEGMENT
	ASSUME CS:FLAT, DS:FLAT, ES:FLAT
	PUBLIC __SEGSTRT_&segname
__SEGSTRT_&segname:
segname ENDS
ENDM

SEGENDMARKER MACRO segname
segname SEGMENT
	PUBLIC __SEGEND_&segname
__SEGEND_&segname LABEL BYTE
segname ENDS
ENDM


TH3216Enter MACRO
   push    ds
   push    es
   push    DATA16
   push    DATA16
   pop     es
   pop     ds
ENDM

TH3216Return MACRO
   pop     es
   pop     ds
   db      66h ; Force next instruction 32Bit
   ret
ENDM

SMPLock MACRO lockptr
   push    ax
@@:
   mov     ax,1
   lock
   xchg    ax,word ptr [lockptr]
   test    ax,ax
   jne     @B
   pop     ax
ENDM

SMPUnLock MACRO lockptr
   mov     word ptr [lockptr],0
ENDM

ThunkStackTo32Stub MACRO StackAllocFn
   LOCAL @@stackok, @@stackerrret
   pushfd
   cli
   SMPLock SMP_Lock
   pop     dword ptr [cpuflags]
   ;check if kernel DS selector DPL == 3; if so, change to 0
   call    LXA_FixSelDPL
   ;allocate private stack
   call    StackAllocFn
   mov     dword ptr [fInitStack], 1
   cmp     eax, 0
   jne     @@stackok
   call    LXA_RestoreSelDPL
   mov     eax, -1                 ;mark failure
	jmp		short @@stackerrret
@@stackok:
   ;stack ptr in eax
   call    dword ptr [intSwitchStack]
@@stackerrret:
	push	dword ptr [cpuflags]
	test	dword ptr [cpuflags],0200h
   SMPUnLock SMP_Lock
	jz		short @F
	sti
@@:
	popfd
   mov     eax, 0                  ;mark success
ENDM

ThunkStackTo16Stub MACRO StackFreeFn
   pushfd
   cli
   SMPLock SMP_Lock
   pop     dword ptr [cpuflags]
   mov     dword ptr [tempeax], eax    ;save eax
   mov     dword ptr [tempesi], esi    ;save esi
   mov     esi, esp
   xor     eax, eax                    ;switch to 16 bits stack
   call    dword ptr [intSwitchStack]
   mov     eax, esi
   call    StackFreeFn
   call    LXA_RestoreSelDPL
   mov     eax, dword ptr [tempeax]
   mov     esi, dword ptr [tempesi]
	push	dword ptr [cpuflags]
   test    dword ptr [cpuflags],0200h
   SMPUnLock SMP_Lock
   jz      short @F
   sti
@@:
	popfd
ENDM

ThunkStackTo32_Int MACRO
   LOCAL @@cont32
	pushfd
   cmp     dword ptr [fInitStack], 0
   je      @@cont32
;;;;        int     3
   cli
   SMPLock SMP_Lock
   pop     dword ptr [cpuflags]
   mov     dword ptr [tempeax], eax    ;save eax
   call    LXA_FixSelDPL               ; (SM)
   pop     eax                         ;pop saved flat stack pointer
   ;and switch back to our flat stack again
   call    dword ptr [intSwitchStack]
   mov     eax, dword ptr [tempeax]    ;restore eax
	push	dword ptr [cpuflags]
   test    dword ptr [cpuflags],0200h
   SMPUnLock SMP_Lock
   jz      short @@cont32
   sti
@@cont32:
	popfd
ENDM

ThunkStackTo16_Int MACRO
   LOCAL @@cont16
   pushfd
   cmp     dword ptr [fInitStack], 0
   je      @@cont16
   cli
   SMPLock SMP_Lock
   pop     dword ptr [cpuflags]
   mov     dword ptr [tempeax], eax    ;save eax
   mov     dword ptr [tempesi], esi    ;save esi
   mov     esi, esp
   xor     eax, eax    ;switch to 16 bits stack
   call    dword ptr [intSwitchStack]
   push    esi     ;flat stack ptr (popped before switching back to flat)
   call    LXA_RestoreSelDPL           ; (SM)
   mov     eax, dword ptr [tempeax]
   mov     esi, dword ptr [tempesi]
	push	dword ptr [cpuflags]
   test    dword ptr [cpuflags],0200h
   SMPUnLock SMP_Lock
   jz      short @@cont16
   sti
@@cont16:
	popfd
ENDM

ThunkStackTo32 MACRO
	push	OFFSET FLAT:LXA_StackAlloc
	call	LXA_SwitchStackTo32
ENDM

ThunkStackTo16 MACRO
	push	OFFSET FLAT:LXA_StackFree
	call	LXA_SwitchStackTo16
ENDM

ThunkIRQStackTo32 MACRO
	push	OFFSET FLAT:LXA_IrqStackAlloc
	call	LXA_SwitchStackTo32
ENDM

ThunkIRQStackTo16 MACRO
	push	OFFSET FLAT:LXA_IrqStackFree
	call	LXA_SwitchStackTo16
ENDM

ThunkTaskStackTo32 MACRO
	push	OFFSET FLAT:_LXA_TaskStackAlloc
	call	LXA_SwitchStackTo32
ENDM

ThunkTaskStackTo16 MACRO
	push	OFFSET FLAT:_LXA_TaskStackFree
	call	LXA_SwitchStackTo16
ENDM

ThunkUtilStackTo32 MACRO
	push	OFFSET FLAT:LXA_UtilStackAlloc
	call	LXA_SwitchStackTo32
ENDM

ThunkUtilStackTo16	MACRO
	push	OFFSET FLAT:LXA_UtilStackFree
	call	LXA_SwitchStackTo16
ENDM

ThunkCall MACRO EntryPoint
	ThunkStackTo32
	cmp		eax,0
	jne		@F
	call	EntryPoint
	ThunkStackTo16
@@:
ENDM

ThunkCallIRQ MACRO EntryPoint
	ThunkIRQStackTo32
	cmp		eax,0
	jne		@F
	call	EntryPoint
	ThunkIRQStackTo16
@@:
ENDM

