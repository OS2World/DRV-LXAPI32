; $Id: lxstack.asm,v 1.15 2005/04/18 22:28:34 smilcke Exp $

;
; lxrmcall.asm
; Autor:				Stefan Milcke
; Erstellt	am:			29.03.2003
; Letzte Aenderung	am:	15.07.2004
;
		.386p
		include defcfg.inc
		INCL_DOS		equ	1
		INCL_DOSERRORS	equ	1
		include	os2.inc
		include	seg32.inc
		include devhlp32.inc
		include lxbasem.inc
		include lxbases.inc
		include lxextdef.inc
		include infoseg.inc

;##############################################################################
;*	Constants
;##############################################################################
MAX_NUM_IRQSTACK	equ	(4)
IFDEF CONFIG_THREAD_SIZE
STACKSIZE			equ (CONFIG_THREAD_SIZE)
ELSE
STACKSIZE			equ	(2 * 4096)
ENDIF

STACKCALC			equ	(STACKSIZE)

;VMFLAGS			equ (VMDHA_FIXED + VMDHA_CONTIG + VMDHA_ALIGN64K)
;VMFLAGS			equ (VMDHA_FIXED + VMDHA_ALIGN64K)
;VMFLAGS			equ (VMDHA_FIXED + VMDHA_CONTIG)
VMFLAGS			equ (VMDHA_FIXED)
;VMFLAGS			equ	(0)
VMFLAGSFIRST		equ (VMDHA_USEHIGHMEM)

;MAX_ALLOC_ALIGNED_RETRY	equ	250
MAX_ALLOC_ALIGNED_RETRY	equ	500
ALLOC_ALIGNED_ALIGN		equ	STACKSIZE

;##############################################################################
;*	DATA32
;##############################################################################
DATA32 SEGMENT
		extrn _lx_StackRoot : DWORD
		extrn _lx_IrqStackRoot : DWORD
		extrn _lx_UtlStackRoot : DWORD
		extrn _TKSSBase : DWORD
		extrn	intSwitchStack : DWORD
		extrn	LXA_FixSelDPL : NEAR
		extrn	LXA_RestoreSelDPL : NEAR
		public	_lx_max_alloc_aligned_retry
_lx_max_alloc_aligned_retry	dd MAX_ALLOC_ALIGNED_RETRY
DATA32	ENDS

;##############################################################################
;*	CODE16
;##############################################################################
CODE16	SEGMENT
		ASSUME cs:CODE16, ds:DATA16
		extrn DevHelpFnPtr : dword
		ALIGN	2
thunk3216_stack:
		push	ds
		push	DATA16
		pop		ds
		call	dword ptr ds:DevHelpFnPtr
		pop		ds
		jmp		far	ptr	FLAT:thunk1632_stack
CODE16	ENDS

;##############################################################################
;*	TEXT32
;##############################################################################
TEXT32 SEGMENT
		ASSUME	CS:FLAT, DS:FLAT, ES:FLAT
		ALIGN 4
		public	LXA_StackThunkDevHlp
LXA_StackThunkDevHlp	proc near
		push	edi
		push	ecx
		jmp		far	ptr	CODE16:thunk3216_stack
thunk1632_stack:
		pop		ecx
		pop		edi
		ret
LXA_StackThunkDevHlp	endp

;******************************************************************************
; _LXA_VMAllocHelper
; IN:	ecx	=	size
;		edx =	flags
; OUT:	eax =	allocated storage
;******************************************************************************
LXA_VMAllocHelper	proc near
		extrn	_lx_default_malloc_flags : DWORD
		push	esi
		push	edi
		push	edx
		mov		edi,-1
		mov		eax,edx
		test	_lx_default_malloc_flags,VMDHA_USEHIGHMEM
		jz		LXA_VMAllocHelper1
		test	eax,VMDHA_16M
		jnz		LXA_VMAllocHelper1
		or		eax,VMFLAGSFIRST
		mov		dl,DevHlp_VMAlloc
		call	LXA_StackThunkDevHlp
		jnc		LXA_VMAllocHelper3
		pop		eax
		push	eax
LXA_VMAllocHelper1:
		mov		dl,DevHlp_VMAlloc
		call	LXA_StackThunkDevHlp
LXA_VMAllocHelper2:
		jnc		LXA_VMAllocHelper3
		mov		eax,0
LXA_VMAllocHelper3:
		pop		edx
		pop		edi
		pop		esi
		ret
LXA_VMAllocHelper	endp

;******************************************************************************
; _LXA_VMFreeNoStackThunk
; IN:	eax	=	memory storage
;******************************************************************************
		ALIGN	4
		public	LXA_VMFreeNoStackThunk
LXA_VMFreeNoStackThunk	proc near
		push	edx
		push	esi
		push	edi
		mov		dl,DevHlp_VMFree
		call	LXA_StackThunkDevHlp
		pop		edi
		pop		esi
		pop		edx
		ret
LXA_VMFreeNoStackThunk	endp

;******************************************************************************
; LXA_VMAllocAlignedNoStackThunk
; IN:	ecx	=	size
;		edx =	flags
; OUT:	eax =	allocated storage
;******************************************************************************
		ALIGN	4
LXA_VMAllocAlignedNoStackThunk	proc near
		cmp		ecx,4096
		jae		@F
		jmp		LXA_VMAllocHelper
@@:	push	esi
		push	ebx
		push	edx
		mov		esi,_lx_max_alloc_aligned_retry
@@:	call	LXA_VMAllocHelper
		jc		@F
		mov		ebx,eax
		and		ebx,NOT (ALLOC_ALIGNED_ALIGN-1)
		cmp		eax,ebx
		je		@F

		call	LXA_VMFreeNoStackThunk
		push	ecx
		mov		ecx,4096
		call	LXA_VMAllocHelper
		pop		ecx
		jc		@F

		push	eax
		dec		esi
		jnz		@B
@@:	cmp		esi,0
		jne		@F
		xor		eax,eax
@@:	cmp		esi,_lx_max_alloc_aligned_retry
		je		@F
		mov		ebx,eax
		pop		eax
		call	LXA_VMFreeNoStackThunk
		mov		eax,ebx
		inc		esi
		jmp		short @B
@@:	cmp		eax,0
		jne		@F
		add		ecx,ALLOC_ALIGNED_ALIGN / 2
		call	LXA_VMAllocHelper
@@:	pop		edx
		pop		ebx
		pop		esi
		ret
LXA_VMAllocAlignedNoStackThunk	endp

;******************************************************************************
; LXA_Align
; IN:	eax =	memory storage
; OUT:	eax =	aligned memory storage
;******************************************************************************
		ALIGN	4
LXA_AlignMemory	proc near
		push	ebx
		mov		ebx,eax
		and		ebx,NOT (ALLOC_ALIGNED_ALIGN-1)
		cmp		eax,ebx
		je		@F
		add		eax,(ALLOC_ALIGNED_ALIGN-1)
		and		eax,NOT (ALLOC_ALIGNED_ALIGN-1)
@@:	pop		ebx
		ret
LXA_AlignMemory	endp

;******************************************************************************
; _LXA_VMAllocAligned
; IN:	[ebp+ 8]	=	flags
;		[ebp+12] 	=	size
;		[ebp+16]	=	&aligned_mem
;		[ebp+20]	=	&real_size
;		[ebp+24]	=	&real_mem
; OUT:	eax			=	aligned memory or 0 on error
;******************************************************************************
		public	_LXA_VMAllocAligned
_LXA_VMAllocAligned proc near
		push	ebp
		mov		ebp,esp
		push	ecx
		push	edx
		mov		edx,[ebp+ 8]
		mov		ecx,[ebp+12]
		ThunkStackTo16_Int
		call	LXA_VMAllocAlignedNoStackThunk
		ThunkStackTo32_Int
		cmp		eax,0
		je 		_LXA_VMAllocAlignedError
		mov		edx,[ebp+20]
		mov		[edx],ecx
		mov		edx,[ebp+24]
		mov		[edx],eax
		cmp		ecx,4096
		jb		@F
		call	LXA_AlignMemory
@@:	mov		edx,[ebp+16]
		mov		[edx],eax
_LXA_VMAllocAlignedError:
		pop		edx
		pop		ecx
		pop		ebp
		ret
_LXA_VMAllocAligned endp


;******************************************************************************
; LXA_VMStackAlloc
; IN:	ecx	=	size
; OUT:	eax =	allocated storage
;******************************************************************************
		ALIGN	4
		public	LXA_VMStackAlloc
LXA_VMStackAlloc	 proc near
		push	edx
		mov		edx,VMFLAGS
		call	LXA_VMAllocHelper
		pop		edx
		jnc		short @F
		xor		eax,eax
@@:	ret
LXA_VMStackAlloc	 endp

;******************************************************************************
; LXA_StackFindFreeEntry
; IN:	edi =	offset of first PDDStack structure
; OUT:	eax =	Stack base
;******************************************************************************
		ALIGN	4
LXA_StackFindFreeEntry	proc near
		; next ptr valid?
@@1:	cmp		dword ptr [edi].PDDStack_Next,0
		jne		@@2
		; invalid ptr, allocate a new PDDStack structure
		mov		ecx,SIZE PDDStack
		call	LXA_VMStackAlloc
		cmp		eax,0
		je		@@4
		; wipe out new allocated PDDStack structure
		push	eax
;		push	ecx
		push	edi
		mov		edi,eax
		mov		ecx,SIZE PDDStack
		xor		al,al
		cld
		rep		stosb
		pop		edi
;		pop		ecx
		pop		eax
		; Link in new allocated PDDStack structure
		mov		dword ptr [edi].PDDStack_Next,eax
		; Get next PDDStack structure
@@2:	mov		edi,dword ptr [edi].PDDStack_Next
		test	dword ptr [edi].PDDStack_Flags,PDDSTACK_USED
		jnz		@@1
		; Stack is unused, so check, if Stack ptr is valid
		mov		eax,dword ptr [edi].PDDStack_Base
		cmp		eax,0
		jne		@@3
		; Stack ptr invalid, allocate a new one
		mov		ecx,STACKSIZE
		push	edx
		mov		edx,VMFLAGS ;+ VMDHA_USEHMA
		call	LXA_VMAllocAlignedNoStackThunk
		pop		edx
		call	LXA_AlignMemory
		mov		dword ptr [edi].PDDStack_Base,eax
		cmp		eax,0
		je		@@4
		; wipe out first bytes of new stack (this is the thread_info structure!)
		push	eax
;		push	ecx
		push	edi
		mov		edi,eax
		xor		eax,eax
		mov		ecx,52
		cld
		rep		stosb
		pop		edi
;		pop		ecx
		pop		eax
		; adjust pointer to top of stack
@@3:	add		eax,(STACKCALC)
		or		dword ptr [edi].PDDStack_Flags,PDDSTACK_USED
		ret
@@4:	int 3	; This is fatal!
		mov		eax,0
		ret
LXA_StackFindFreeEntry	endp

;******************************************************************************
; LXA_StackAlloc
; IN:	
; OUT:	eax	=	allocated stack storage
;******************************************************************************
		ALIGN	4
		public	LXA_StackAlloc
LXA_StackAlloc	proc near
		push	ecx
		push	edi
		mov		edi,offset FLAT:_lx_StackRoot
		call	LXA_StackFindFreeEntry
		pop		edi
		pop		ecx
		ret
LXA_StackAlloc	endp

;******************************************************************************
; LXA_StackFree
; IN:	eax =	stack
; OUT:	eax =	0, OK, else error
;******************************************************************************
		ALIGN	4
		public	LXA_StackFree
LXA_StackFree		proc near
		sub		eax,(STACKCALC)
		push	edi
		mov		edi,offset FLAT:_lx_StackRoot
@a1:	mov		edi,dword ptr [edi].PDDStack_Next
		cmp		edi,0
		je		@b1
		cmp		eax,dword ptr [edi].PDDStack_Base
		jne		@a1
		and		dword ptr [edi].PDDStack_Flags,NOT DWORD (PDDSTACK_USED)
		xor		eax,eax
@b1:	pop		edi
		ret
LXA_StackFree		endp

;LXA_USE_CURRENT_THREAD_INFO	equ	1
IFDEF LXA_USE_CURRENT_THREAD_INFO
		extrn	_lx_current_thread_info : DWORD
		extrn	privateStack : DWORD
		ALIGN	4
LXA_CompareStacks	proc near
		push	edx
		mov		edx,dword ptr _lx_current_thread_info
		cmp		edx,0
		je		@F
		cmp		eax,edx
		jl		@F
		add		edx,(STACKSIZE-100)
		cmp		eax,edx
		jge		@F
		pop		edx
		clc
		ret
@@:	pop		edx
		stc
		ret
LXA_CompareStacks endp
ENDIF

;******************************************************************************
; LXA_IrqStackAlloc
; IN:	
; OUT:	eax	=	allocated stack storage
;******************************************************************************
		ALIGN	4
		public	LXA_IrqStackAlloc
LXA_IrqStackAlloc		proc near
IFDEF LXA_USE_CURRENT_THREAD_INFO
		mov		eax,dword ptr [privateStack]
		mov		eax,[eax]
		call	LXA_CompareStacks
		jc		@F
		ret
ENDIF
@@:	push	ecx
		push	edi
		mov		edi,offset FLAT:_lx_IrqStackRoot
		call	LXA_StackFindFreeEntry
@@:	pop		edi
		pop		ecx
		ret
LXA_IrqStackAlloc		endp

;******************************************************************************
; LXA_IrqStackFree
; IN:	eax =	stack
; OUT:	eax =	0, OK, else error
;******************************************************************************
		ALIGN	4
		public	LXA_IrqStackFree
LXA_IrqStackFree		proc near
IFDEF LXA_USE_CURRENT_THREAD_INFO
		call	LXA_CompareStacks
		jc		@F
		ret
ENDIF
@@:	sub		eax,(STACKCALC)
		push	edi
		mov		edi,offset FLAT:_lx_IrqStackRoot
@a2:	mov		edi,dword ptr [edi].PDDStack_Next
		cmp		edi,0
		je		@b2
		cmp		eax,dword ptr [edi].PDDStack_Base
		jne		@a2
		and		dword ptr [edi].PDDStack_Flags,NOT DWORD (PDDSTACK_USED)
		xor		eax,eax
@b2:	pop		edi
		ret
LXA_IrqStackFree		endp

;******************************************************************************
; LXA_UtilStackFindEntry
; OUT:	eax =	Pointer to UTLStack structure, 0 if not found
;******************************************************************************
		ALIGN	4
		extrn	_lx_InfoSegLDTPtr : ABS
LXA_UtilStackFindEntry	proc near
		push	ebx
		push	ecx
		mov		eax,_lx_InfoSegLDTPtr
		mov		eax,dword ptr [eax]
		mov		bx,word ptr [eax].LIS_CurProcID	; PID now in bx
		mov		cx,word ptr [eax].LIS_CurThrdID ; TID now in cx

		mov		eax,dword ptr [_lx_UtlStackRoot]
		cmp		eax,0
		je		@UA_RET
@UA_LOOP:
		cmp		bx,word ptr [eax].UTLStack_PID
		jne		@UA_CONT
		cmp		cx,word ptr [eax].UTLStack_TID
		je		@UA_RET
@UA_CONT:
       mov		eax,dword ptr [eax]
		cmp		eax,0
		jne		@UA_LOOP
@UA_RET:
		pop		ecx
		pop		ebx
		cmp		eax,0
		ret
LXA_UtilStackFindEntry	endp
;******************************************************************************
; LXA_UtilStackAlloc
; IN:	
; OUT:	eax	=	allocated stack storage
;******************************************************************************
		ALIGN	4
		public	LXA_UtilStackAlloc
LXA_UtilStackAlloc	proc near
		call	LXA_UtilStackFindEntry
		jne		@UA_STACKFOUND
@UA_STD_STACKALLOC:
		jmp		LXA_StackAlloc
@UA_STACKFOUND:
		test	dword ptr [eax].UTLStack_Flags,UTLSTACK_USED
		jnz		@UA_STD_STACKALLOC
		or		dword ptr [eax].UTLStack_Flags,UTLSTACK_USED
		mov		eax,dword ptr [eax].UTLStack_Base
		add		eax,(STACKCALC)
		ret
LXA_UtilStackAlloc	endp

;******************************************************************************
; LXA_UtilStackFree
; IN:	eax =	stack
; OUT:	eax =	0, OK, else error
;******************************************************************************
		ALIGN	4
		public	LXA_UtilStackFree
LXA_UtilStackFree	proc near
		push	ebx
		mov		ebx,eax
		call	LXA_UtilStackFindEntry
		jne		@UF_STACKFOUND
@UF_STD_STACKFREE:
		mov		eax,ebx
		pop		ebx
		jmp	LXA_StackFree
@UF_STACKFOUND:
		sub		ebx,(STACKCALC)
		cmp		dword ptr [eax].UTLStack_Base,ebx
		jne		@UF_STD_STACKFREE
		test	dword ptr [eax].UTLStack_Flags,UTLSTACK_USED
		jz		@UF_STD_STACKFREE
		and		dword ptr [eax].UTLStack_Flags,NOT UTLSTACK_USED
		pop		ebx
		ret
LXA_UtilStackFree	endp

;******************************************************************************
; LXA_SwitchStackTo32
; IN:	alloc fn pushed on the stack
; OUT:	eax =	0, OK, -1 error
;******************************************************************************
		ALIGN	4
		public	LXA_SwitchStackTo32
LXA_SwitchStackTo32	proc near
		push	esi
		mov		esi,esp
		; Stack layout
		; esi+4		return address
		; esi+8		alloc fn ptr
		pushfd
		cli
		SMPLock	SMP_Lock
		call	LXA_FixSelDPL
		mov		eax,dword ptr ss:[esi+8]; alloc fn
		call	eax
		mov		dword ptr [fInitStack],1
		cmp		eax,0
		je		@@LXA_SST32_err
		; store saved regs and return addr on the new stack
		pop		esi						; saved flags
		mov		dword ptr FLAT:[eax-12],esi
		pop		esi						; saved esi
		mov		dword ptr FLAT:[eax- 8],esi
		pop		esi						; return addr
		mov		dword ptr FLAT:[eax- 4],esi
		pop		esi						; alloc fn (discarded)
		sub		eax,12	; Adjust stack
		call	dword ptr [intSwitchStack]
		SMPUnLock	SMP_Lock
		pop		eax
		push	eax
		test	eax,0200h
		jz		@F
		sti
@@:	popfd
		pop		esi
		xor		eax,eax
		ret
@@LXA_SST32_err:
		SMPUnLock	SMP_Lock
		pop		eax
		push	eax
		test	eax,0200h
		jz		@F
		sti
@@:	popfd
		pop		esi
		pop		eax		; return addr
		add		esp,4	; adjust stack (discard alloc fn)
		push	eax
		mov		eax,-1
		ret
LXA_SwitchStackTo32	endp

;******************************************************************************
; LXA_SwitchStackTo16
; IN:	free fn pushed on the stack
; OUT:
;******************************************************************************
		ALIGN	4
		public	LXA_SwitchStackTo16
LXA_SwitchStackTo16	proc near
		push	esi			; esi+0
		mov		esi,esp
		; Stack layout
		; esi+4		return address
		; esi+8		free fn ptr
		push	eax			; esi- 4
		pushfd				; esi- 8
		cli
		SMPLock	SMP_Lock
		xor		eax,eax
		call	dword ptr [intSwitchStack]
		call	LXA_RestoreSelDPL
		mov		eax,esi
		push	dword ptr [eax+ 4]	; return addr
		push	dword ptr [eax+ 0]	; saved esi
		push	dword ptr [eax- 4]	; saved eax
		push	dword ptr [eax- 8]	; saved flags
		mov		esi,dword ptr [eax+8]	; free fn
		add		eax,12
		call	esi
		SMPUnLock	SMP_Lock
		pop		eax
		push	eax
		test	eax,0200h
		jz		@F
		sti
@@:	popfd
		pop		eax
		pop		esi
		ret
LXA_SwitchStackTo16	endp

TEXT32	ENDS

end

