; $Id: lxstartup.asm,v 1.70 2006/04/27 21:17:52 smilcke Exp $

;
; lxstartup.asm
; Autor:				Stefan Milcke
; Erstellt	am:			08.12.2001
; Letzte Aenderung	am:	16.07.2004
;
		.386p
		include defcfg.inc
		include	seg32.inc

		INCL_DOS		equ	1
		INCL_DOSERRORS	equ	1
		include	os2.inc
		include	r0thunk.inc
		include	rp.inc
		include	rm.inc
		include	devhlp32.inc
		include	infoseg.inc
		include	lxbases.inc
		include	lxbasem.inc
		include lxextdef.inc
INCL_DEF =	1
		include	basemaca.inc

MAX_GDTSELECTORS		EQU	64

; Status word masks
STERR		EQU	8000H		; Bit 15 - Error
STINTER	EQU	0400H		; Bit 10 - Interim character
STBUI		EQU	0200H		; Bit  9 - Busy
STDON		EQU	0100H		; Bit  8 - Done
STECODE	EQU	00FFH		; Error	code

TDAIRQ_CLAIM	EQU		10000H
TDAIRQ_EOI		EQU		20000H

;##############################################################################
;*	DATA16
;##############################################################################
DATA16	segment
		public DevHelpFnPtr
		public INIT_COUNT
		public GDTSelectors
		public ddTable
		extrn __SEGEND_DATA16 : byte
		extrn _TKSSBase : dword

DevHelpFnPtr	dd 0
GDTSelectors	dw[MAX_GDTSELECTORS] dup (0)
ddTable		dw[6] dup (0)
INIT_COUNT		dw 0
InitPktSeg		dw 0
InitPktOff		dw 0
ulFileAction	dw 0
				dw 0
Irqs_Disabled	dw 0
DevHdrName		db "LXAPIH$", 0
DaemonName		db "LXAPID.EXE",0

DATA16	ends

;##############################################################################
;*	CODE16
;##############################################################################
CODE16	segment
		assume cs:CODE16, ds:DATA16
		extrn DOSOPEN		: far
		extrn DOSWRITE		: far
		extrn DOSCLOSE		: far
		extrn __SEGEND_CODE16 : byte

;******************************************************************************
;*	Strategy routines														   *
;******************************************************************************
;-----------------------------	daemon_stub_strategy ----------------------------
		ALIGN	2
		public	daemon_stub_strategy
daemon_stub_strategy proc far
		mov		ax,	DATA16
		mov		ds,	ax

		movzx	eax, byte ptr es:[bx].reqCommand
		cmp		eax, 0
		jz		short @@daemon_init
		cmp		eax,01fh
		je		short @F
		cmp		eax,01ch
		jne		short @@daemon_do_strat
@@:
		mov		ax,STDON
		jmp		short @@daemon_ret
@@daemon_do_strat:
		cmp		word ptr INIT_COUNT,0
		je		@F
		cmp		word ptr INIT_COUNT,4
		jl		short daemon_err	; return error until device	is ready
@@:
		push	bx
		push	es

		; convert req. packet pointer to a linear adress
		xor		esi,esi
		mov		si,bx
		mov		ax,es
		mov		dl,DevHlp_VirtToLin
		call	dword ptr ds:DevHelpFnPtr
		mov		ebx,eax

		call	far	ptr	FLAT:STRATEGY_			; 32 bits strategy entry point

		pop		es
		pop		bx			   ; restore bx	ptr

@@daemon_ret:
		mov		word ptr es:[bx].reqStatus,	ax	; status code
		ret

@@daemon_init:
		 ;
		 ; DEVICE= initialization
		 ;
		mov		eax,dword ptr es:[bx].i_devHelp
		mov		dword ptr DevHelpFnPtr, eax
		mov		word ptr es:[bx].o_codeend,offset __SEGEND_CODE16
		mov		word ptr es:[bx].o_dataend,offset __SEGEND_DATA16
		mov		ax,STDON
		jmp		short @@daemon_ret

daemon_err:
		mov		ax,	STDON +	STERR +	ERROR_I24_BAD_COMMAND
		jmp		short @@daemon_ret
daemon_stub_strategy endp

;------------------------------ help_stup_strategy	-----------------------------
		public	help_stub_strategy
help_stub_strategy	proc far
		mov		ax,	DATA16
		mov		ds,	ax

		movzx	eax, byte ptr es:[bx].reqCommand
		cmp		eax, 0				; Init
		je		short @@help_init
		cmp		eax, 0Dh			; DosOpen
		jne		short @@help_error
;DosOpen
		cmp		word ptr INIT_COUNT,0
		je		short @@help_ret_ok		; not ours
		push	bx			   ; later we need this
		push	es

		; convert req. packet pointer to a linear adress
		mov		ax,	word ptr InitPktSeg
		xor		esi,esi
		mov		si,word	ptr	InitPktOff
		mov		dl,DevHlp_VirtToLin
		call	dword ptr ds:DevHelpFnPtr
		mov		ebx,eax

		call	far	ptr	FLAT:STRATEGY_

		pop		es
		pop		bx			   ; restore bx	ptr
@@help_ret:
		mov		word ptr es:[bx].reqStatus,ax
		ret
@@help_init:
		mov		eax, dword ptr es:[bx].i_devHelp
		mov		dword ptr DevHelpFnPtr,	eax
		mov		word ptr es:[bx].o_codeend,offset __SEGEND_CODE16
		mov		word ptr es:[bx].o_dataend,offset __SEGEND_DATA16
		call	init_data
		jc		short @@help_error
@@help_ret_ok:
		mov		ax,	STDON
		jmp		short @@help_ret
@@help_error:
		mov		ax,	STDON +	STERR +	ERROR_I24_BAD_COMMAND
		jmp		short @@help_ret
help_stub_strategy	endp

;-----------------------------	device_stub_strategy ----------------------------
		ALIGN	2
		public	device_stub_strategy
device_stub_strategy proc far
		mov		ax,	DATA16
		mov		ds,	ax

		movzx	eax, byte ptr es:[bx].reqCommand
		cmp		eax, 0
		jz		short @@init

		push	bx
		push	es

		; convert req. packet pointer to a linear adress
		xor		esi,esi
		mov		si,bx
		mov		ax,es
		mov		dl,DevHlp_VirtToLin
		call	dword ptr ds:DevHelpFnPtr
		mov		ebx,eax

		call	far	ptr	FLAT:STRATEGY_			; 32 bits strategy entry point

		pop		es
		pop		bx			   ; restore bx	ptr
@@device_ret:
		mov		word ptr es:[bx].reqStatus,	ax	; status code
		ret

@@init:
		 ;
		 ; DEVICE= initialization
		 ;
		mov		word ptr InitPktSeg, es
		mov		word ptr InitPktOff, bx
		mov		word ptr INIT_COUNT,1

		call	device_init

		mov		word ptr es:[bx].o_codeend,offset __SEGEND_CODE16
		mov		word ptr es:[bx].o_dataend,offset __SEGEND_DATA16
		jmp		short @@device_ret

device_stub_strategy endp

;------------------------------ util_stub_strategy -----------------------------
		ALIGN	2
		public	util_stub_strategy
util_stub_strategy	proc far
		mov		ax, DATA16
		mov		ds, ax

		movzx	eax, byte ptr es:[bx].reqCommand
		cmp		eax, 0
		jz		short @@util_init
@@util_do_strat:
		push	bx
		push	es

		; convert req. packet pointer to a linear adress
		xor		esi,esi
		mov		si,bx
		mov		ax,es
		mov		dl,DevHlp_VirtToLin
		call	dword ptr ds:DevHelpFnPtr
		mov		ebx,eax

		call	far	ptr	FLAT:USTRATEGY_			; 32 bits strategy entry point

		pop		es
		pop		bx			   ; restore bx	ptr

@@util_ret:
		mov		word ptr es:[bx].reqStatus,	ax	; status code
		ret

@@util_init:
		 ;
		 ; DEVICE= initialization
		 ;
		mov		eax,dword ptr es:[bx].i_devHelp
		mov		dword ptr DevHelpFnPtr, eax
		mov		word ptr es:[bx].o_codeend,offset __SEGEND_CODE16
		mov		word ptr es:[bx].o_dataend,offset __SEGEND_DATA16
		mov		ax,STDON
		jmp		short @@util_ret

util_err:
		mov		ax,	STDON +	STERR +	ERROR_I24_BAD_COMMAND
		jmp		short @@util_ret
util_stub_strategy endp

;******************************************************************************
;*	IDC	handler																   *
;******************************************************************************
;in: cx = cmd
;	  bx = lower 16	bits of	ULONG parameter
;	  dx = upper 16	bits of	ULONG parameter
;return value in dx:ax
		ALIGN	2
		public	device_stub_idc
device_stub_idc proc far
		enter	0,0
		and		sp,	0fffch		; align	stack
		shl		edx,16
		mov		dx,bx
		push	ds
		push	DATA16
		pop		ds
		call	far	ptr	FLAT:IDC_
		pop		ds
		mov		dx,ax
		shr		eax,16
		xchg	ax,dx
		leave
		retf
device_stub_idc endp

;*******************************************************************************
;*	IRQHandler16																*
;*	On Entry: EAX=irq															*
;*******************************************************************************
		ALIGN	2
IRQHandler16 proc far
		cli		; Disable interrupts
		call	FAR	PTR	FLAT:IRQHandler32
		push	DATA16
		pop		ds
		; Note:	After IRQ the register EAX should contain the flags
		; TDAIRQ_EOI and TDAIRQ_CLAIM
		; If TDAIRQ_EOI	is set,	the	upper 16 bits should contain the irq number
		test	eax,TDAIRQ_EOI
		jz		@F
		; disable interrupts to	prevent	nested interrupt processing
		; in shared	IRQ	environment
		cli

		push	eax
		mov		dl,DevHlp_EOI
		call	dword ptr DevHelpFnPtr
		pop		eax
@@:	test	eax,TDAIRQ_CLAIM
		jnz		@F
		stc
		retf
@@:	clc
		retf
IRQHandler16 endp

;*******************************************************************************
;*	TimerHandler16																*
;*******************************************************************************
		ALIGN	2
TimerHandler16	proc far
; ESI=Stack ptr of interrupted task?
;		push	ebx
;		push	edx
;		push	ds
;		push	DATA16
;		pop		ds
;		xor		ebx,ebx	; IRQ0 = Timer interrupt
;		call	FAR	PTR	FLAT:IRQHandler32
;		pop		ds
;		pop		edx
;		pop		ebx
;		retf

		pushad
		push	ds
		push	es
		push	fs
		push	gs
		push	DATA16
		pop		ds
		xor		ebx,ebx	; IRQ0 = Timer interrupt
		cmp		Irqs_Disabled,0
		jne		@F
		call	FAR	PTR	FLAT:IRQHandler32
@@:	pop		gs
		pop		fs
		pop		es
		pop		ds
		popad
		retf
TimerHandler16	endp

;*******************************************************************************
;*	DevHelp	thunking															*
;*******************************************************************************
		ALIGN	2
thunk3216_devhelp:
		push	ds
		push	DATA16
		pop		ds
		call	dword ptr ds:DevHelpFnPtr
		pop		ds
		jmp		far	ptr	FLAT:thunk1632_devhelp

		ALIGN	2
thunk3216_devhelp_modified_ds:
		push	gs
		push	DATA16
		pop		gs
		call	dword ptr gs:DevHelpFnPtr
		pop		gs
		jmp		far	ptr	FLAT:thunk1632_devhelp

;---------------------------------- init_data ----------------------------------
		ALIGN	2
init_data proc	near
		; TODO:	Check for errors after DevHelp
		; Do some additional inits
		pushad
		push	ds
		push	es
		push	fs
		push	gs

		push	ds
		pop		es

		; Allocate GDT Selectors
		mov		cx,MAX_GDTSELECTORS
		mov		ax,ds
		mov		bx,offset GDTSelectors
		mov		dx,bx
		mov		di,dx
		mov		es,ax
		mov		dl,DevHlp_AllocGDTSelector
		call	dword ptr DevHelpFnPtr

		pop		gs
		pop		fs
		pop		es
		pop		ds
		popad
		ret
init_data endp


;-----------------------------	get16_path_from_arg	-----------------------------
		ALIGN	2
get16_path_from_arg proc near
		; ds:si	-> source
		; es:di	-> destination
		; on return	ds:di points to	after the last backslash
		push	di
		cld
@@:	lodsb
		stosb
		test	al,al
		je		short @F
		cmp		al,' '
		jne		short @B
@@:	pop		ax
@@:	cmp		di,ax
		je		short @F
		cmp		byte ptr es:[di],'\'
		je		short @F
		dec		di
		jmp		short @B
@@:	cmp		di,ax
		je		short @F
		inc		di
@@:	ret
get16_path_from_arg endp

;---------------------------------	start_daemon --------------------------------
		ALIGN	2
		extrn DOSEXECPGM	   : far
start_daemon proc near
		push	es
		push	bx
		push	si
		push	di
		push	bp
		mov		bp,sp
		sub		sp,300

		push	es
		push	ds
		mov		es,	word ptr InitPktSeg
		mov		bx,	word ptr InitPktOff
		mov		si,	word ptr es:[bx].i_initArgs
		mov		ds,	word ptr es:[bx+2].i_initArgs
		push	ss
		pop		es
		lea		di,-300[bp]	; Daemonfile
		call	get16_path_from_arg
		pop		ds
		mov		si,offset DaemonName
@@:	lodsb
		stosb
		test	al,al
		jne		@B
		pop		es

		push	DWORD PTR 0
		push	DWORD PTR 4
		push	DWORD PTR 0
		push	DWORD PTR 0
		lea		dx,-6[bp]	;pRes
		push	ss
		push	dx
		lea		dx,-300[bp]	; Daemonfile
		push	ss
		push	dx
		call	DOSEXECPGM
		mov		sp,bp
		pop		bp
		pop		di
		pop		si
		pop		dx
		pop		es
		ret
start_daemon endp

openclose_devhelper proc near
		enter	24,0
		push	ds
		push	es
		push	bx
		push	si
		push	di
		; bp	  ->  old bp
		; bp - 2  -> FileHandle
		; bp - 4  -> ActionTaken
		; bp - 8  -> IOCTL parm	(4 bytes)
		; bp - 24 -> IOCTL data	(16	bytes)
		; Open LXAPIH$
		push	seg	DATA16				   ; seg  DevHdrName
		push	offset DevHdrName
		push	ss						   ; seg &FileHandle
		lea		ax,	[bp	- 2]
		push	ax						   ; ofs &FileHandle
		push	ss						   ; seg &ActionTaken
		lea		ax,	[bp	- 4]
		push	ax						   ; ofs &ActionTaken
		push	dword ptr 0				   ; file size
		push	0						   ; file attributes
		push	OPEN_ACTION_FAIL_IF_NEW	+ OPEN_ACTION_OPEN_IF_EXISTS
		push	OPEN_SHARE_DENYNONE	+ OPEN_ACCESS_READONLY
		push	dword ptr 0				   ; reserved
		call	DOSOPEN
		cmp		ax,	NO_ERROR
		jne		@F
		push	word ptr [bp - 2]		   ; FileHandle
		call	DOSCLOSE
		xor		ax,ax
		cmp		word ptr [bp - 2],0
		jne		@F
		inc		ax
@@:	pop		di
		pop		si
		pop		bx
		pop		es
		pop		ds
		leave
		cmp		ax,0
		ret
openclose_devhelper endp

;---------------------------------	device_init	---------------------------------
device_init proc near
		enter	24,	0
		push	ds
		push	es
		push	bx
		push	si
		push	di

		call	openclose_devhelper
		jne		short @@error
		inc		word ptr INIT_COUNT
		call	openclose_devhelper
		jne		short @@error
		call	start_daemon
@@:	inc		word ptr INIT_COUNT
		call	openclose_devhelper
		jne		short @@error
		cmp		word ptr INIT_COUNT,0
		je		@@device_init_ret_ok
       cmp		word ptr INIT_COUNT,255
		jne		@B
@@:	mov		bx,OFFSET INIT_COUNT
		mov		ax,DATA16
		xor		di,di
		mov		cx,100
		mov		dl,DevHlp_ProcBlock
		call	dword ptr ds:DevHelpFnPtr
       cmp		word ptr INIT_COUNT,255
		je		@B
@@device_init_ret_ok:
		mov		ax,	STDON
@@device_init_ret:
		pop		di
		pop		si
		pop		bx
		pop		es
		pop		ds
		leave
		ret
@@error:
		mov		ax,	STDON +	STERR +	ERROR_I24_GEN_FAILURE
		jmp		short @@device_init_ret

device_init endp
CODE16	ends

DATA32 SEGMENT
		public __SEGSTRT_DATA32
__SEGSTRT_DATA32 label byte
DATA32	ends

IRQHandler	0
IRQHandler	1
IRQHandler	2
IRQHandler	3
IRQHandler	4
IRQHandler	5
IRQHandler	6
IRQHandler	7
IRQHandler	8
IRQHandler	9
IRQHandler	10
IRQHandler	11
IRQHandler	12
IRQHandler	13
IRQHandler	14
IRQHandler	15

;##############################################################################
;* SEGBEGINMARKERs
;##############################################################################
SEGBEGINMARKER BSS32
SEGBEGINMARKER CONST32_RO

include segbegin.inc

;##############################################################################
;*	TEXT32
;##############################################################################
SEGBEGINMARKER TEXT32

TEXT32 SEGMENT
ASSUME	CS:FLAT, DS:FLAT, ES:FLAT
		extrn  	_IDCEntry : NEAR
		extrn  	DOSIODELAYCNT : ABS
		extrn  	GetTKSSBase : NEAR
		extrn	LXA_SwitchStackTo32 : NEAR
		extrn	LXA_SwitchStackTo16	: NEAR
		extrn  	LXA_StackAlloc :	NEAR
		extrn  	LXA_StackFree : NEAR
		extrn  	LXA_IrqStackAlloc : NEAR
		extrn  	LXA_IrqStackFree	: NEAR
		extrn	LXA_UtilStackAlloc : NEAR
		extrn	LXA_UtilStackFree : NEAR
		public 	__text
		public 	__stext
__text:
__stext:

;******************************************************************************
;*	strategy routine														   *
;******************************************************************************
		extrn  _LX_Strategy	: NEAR
		ALIGN	4
STRATEGY_ proc	far
		mov		eax,DOS32FLATDS
		mov		ds,eax
		mov		es,eax
		cmp		dword ptr [intSwitchStack],0
		jne		@F
		call	GetTKSSBase
@@:
		ThunkStackTo32
		cmp		eax,0
		jne		@F
		mov		eax,ebx
		call	_LX_Strategy
		ThunkStackTo16
@@:
		retf
STRATEGY_ endp

;*******************************************************************************
;* util strategy routine                                                       *
;*******************************************************************************
		extrn	_LX_UStrategy : NEAR
		ALIGN	4
USTRATEGY_	proc far
		mov		eax,DOS32FLATDS
		mov		ds,eax
		mov		es,eax
		cmp		dword ptr [intSwitchStack],0
		jne		@F
		call	GetTKSSBase
@@:
		ThunkUtilStackTo32
		cmp		eax,0
		jne		@F
		mov		eax,ebx
		call	_LX_UStrategy
		ThunkUtilStackTo16
@@:
		retf
USTRATEGY_	endp

;******************************************************************************
;*	IDC	routine																   *
;******************************************************************************
;in: ecx =	cmd
;	  edx =	ULONG parameter
;return value in eax
		ALIGN	4
IDC_ proc far
		push	ds
		push	es
		push	fs
		push	gs
		push	ebx
		mov		eax,DOS32FLATDS
		mov		ds,eax
		mov		es,eax
		ThunkStackTo32
		cmp		eax,0
		jne		@F
		call	_IDCEntry
		ThunkStackTo16
@@:	pop		ebx
		pop		gs
		pop		fs
		pop		es
		pop		ds
		retf
IDC_ endp

;******************************************************************************
;*	irq	handler																   *
;*	INPUT: ebx = irq nr														   *
;******************************************************************************
		extrn  _LX_IRQEntry	: NEAR
		ALIGN	4
IRQHandler32	proc far
		mov		eax,DOS32FLATDS
		mov		ds,eax
		mov		es,eax
		ThunkIRQStackTo32
		cmp		eax,0
		jne		@F
		mov		eax,ebx
		call	_LX_IRQEntry
		ThunkIRQStackTo16
@@:
		retf
IRQHandler32	endp

		PUBLIC	_LXA_DisableIrqs
_LXA_DisableIrqs	proc near
		mov		DATA16:Irqs_Disabled,1
		ret
_LXA_DisableIrqs	endp

;*******************************************************************************
;*	LXA_FixSelDPL																*
;*	Set	DPL	of DOS32FLATDS selector	to 0 or	else we'll get a trap D	when loading*
;*	it into	the	SS register	(DPL must equal	CPL	when loading a selector	into SS)*
;*******************************************************************************
		ALIGN	4
		public	LXA_FixSelDPL
LXA_FixSelDPL	proc near
		cmp		fWrongDPL, 1
		jne		short @@fixdpl_end
		cmp		SelRef,	0
		jne		short @@fixdpl_endfix
		push	eax
		push	ebx
		push	edx
		sgdt	fword ptr [gdtsave]		; access the GDT ptr
		mov		ebx, dword ptr [gdtsave+2]	; get lin addr of GDT
		mov		eax, ds				; build	offset into	table
		and		eax, 0fffffff8h			; mask away	DPL
		add		ebx, eax			; build	address
		mov		eax, dword ptr [ebx+4]
		mov		edx, eax
		shr		edx, 13
		and		edx, 3
		;has the OS/2 kernel finally changed the DPL to	0?
		cmp		edx, 0
		jne		@@changedpl
		mov		fWrongDPL, 0		;don't bother anymore
		mov		SelRef,	0
		jmp		short @@endchange
@@changedpl:
		mov		oldDPL,	eax
		and		eax, NOT 6000h		;clear bits	5 &	6 in the high word (DPL)
		mov		dword ptr [ebx+4], eax
@@endchange:
		pop		edx
		pop		ebx
		pop		eax
@@fixdpl_endfix:
		inc		SelRef
@@fixdpl_end:
		ret
LXA_FixSelDPL	endp

;*******************************************************************************
;*	LXA_RestoreSelDPL															*
;*	Restore	DPL	of DOS32FLATDS selector	or else	OS/2 kernel	code running		*
;*	in ring	3 will trap	(during	booting	only)									*
;*	This sel has a DPL of 0	when PM	starts up									*
;*******************************************************************************
		ALIGN	4
		public	LXA_RestoreSelDPL
LXA_RestoreSelDPL	proc near
		cmp		fWrongDPL, 1
		jne		short @@restdpl_end
		cmp		SelRef,	1
		jne		short @@restdpl_endrest
		push	eax
		push	ebx
		sgdt	fword ptr [gdtsave]		; access the GDT ptr
		mov		ebx, dword ptr [gdtsave+2]	; get lin addr of GDT
		mov		eax, ds				; build	offset into	table
		and		eax, 0fffffff8h			; mask away	DPL
		add		ebx, eax			; build	address
		mov		eax, oldDPL
		mov		dword ptr [ebx+4], eax
		pop		ebx
		pop		eax
@@restdpl_endrest:
		dec		SelRef
@@restdpl_end:
		ret
LXA_RestoreSelDPL	endp

;*******************************************************************************
;*	DevHelp	thunking															*
;*******************************************************************************
;------------------------------------ DevHlp -----------------------------------
		public	DevHlp
		public	_DevHlp	; TARGET_OS2_GNU2
		ALIGN	4
DevHlp	proc	near
_DevHlp:               ; TARGET_OS2_GNU2
		ThunkStackTo16_Int
		jmp		far	ptr	CODE16:thunk3216_devhelp
		ALIGN	4
thunk1632_devhelp:
		ThunkStackTo32_Int
		ret
DevHlp	endp

;------------------------------ DevHlp_ModifiedDS ------------------------------
		public	DevHlp_ModifiedDS
		ALIGN	4
DevHlp_ModifiedDS proc	near
		ThunkStackTo16_Int
		jmp	far	ptr	CODE16:thunk3216_devhelp_modified_ds
DevHlp_ModifiedDS endp

;*******************************************************************************
;*	misc helpers																*
;*******************************************************************************
;---------------------------------- iodelay32_	---------------------------------
		public	iodelay32_
		ALIGN	4
iodelay32_	proc near
		mov	  eax, DOSIODELAYCNT
@@:	dec	  eax
		jnz	  @b
		loop  iodelay32_
		ret
iodelay32_	endp

TEXT32	ends

;##############################################################################
;*	DATA32
;##############################################################################
DATA32 SEGMENT
		public __OffsetFinalCS16
		public __OffsetFinalDS16
		public stackbase
		public stacksel

		extrn	intSwitchStack : dword

		public	gdtsave
		public	fWrongDPL
		public	SelRef
		public	oldDPL
		public	_InitThreadID

gdtsave	dq 0
fWrongDPL	dd 1
SelRef		dd 0
oldDPL		dd 0

stacksel			dd 0
stackbase			dd 0
_InitThreadID		dd 0

		public _TimerHandler_PTR
_TimerHandler_PTR	dw	OFFSET CODE16:TimerHandler16
					dw	SEG	CODE16:TimerHandler16

__OffsetFinalCS16	dw OFFSET CODE16:__SEGEND_CODE16
__OffsetFinalDS16	dw OFFSET DATA16:__SEGEND_DATA16

		public _GDTSelectors_PTR
_GDTSelectors_PTR	dw OFFSET DATA16:GDTSelectors
					dw SEG DATA16:GDTSelectors

					public _INIT_COUNT_PTR
_INIT_COUNT_PTR	dw OFFSET DATA16:INIT_COUNT
					dw SEG DATA16:INIT_COUNT
DATA32	ends

IFDEF TARGET_OS2_ELF
SEGMARKER _init_setup,___setup_start
SEGMARKER __ksymtab,___start___ksymtab
SEGMARKER __ksymtab_gpl,___start___ksymtab_gpl
ENDIF

include segments.inc



end

