; $Id: lxthreads.asm,v 1.11 2005/05/02 23:56:07 smilcke Exp $

;
; lxthreads.asm
; Autor:				Stefan Milcke
; Erstellt	am:			07.11.2004
; Letzte Aenderung	am:	07.11.2004
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
INCL_DEF =	1
		include	basemaca.inc

CODE16	SEGMENT
		extrn	INIT_COUNT		: DWORD
CODE16	ends

DATA32 SEGMENT
		extrn	tempeax 		: WORD
		extrn	tempedx	 		: WORD
		extrn	tempesi 		: WORD
		extrn	cpuflags		: WORD
		extrn	fInitStack		: WORD
		extrn	SMP_Lock		: WORD
		extrn	intSwitchStack	: WORD
_lx_started_task	NTASK<0>
		public	_lx_started_task
DATA32	ends

TEXT32	SEGMENT
ASSUME	CS:FLAT, DS:FLAT, ES:FLAT
		extrn	LXA_SwitchStackTo32 : NEAR
		extrn	LXA_SwitchStackTo16	: NEAR
		extrn  	LXA_StackAlloc		: NEAR
		extrn  	LXA_StackFree		: NEAR
		extrn	LXA_FixSelDPL		: NEAR
		extrn	LXA_RestoreSelDPL	: NEAR
		extrn	VDHCREATETHREAD		: NEAR
		extrn	VDHEXITTHREAD		: NEAR
;******************************************************************************
; _LXA_CreateThread
; IN:	eax =	pointer to thread entry point
;		edx = 	pointer where to store thread id
; OUT:	eax =	If successfull thread id, 0 on error
;******************************************************************************
		ALIGN	4
		public	_LXA_CreateThread
_LXA_CreateThread	proc near
		push	ebx
		push	ecx
		push	edx
		ThunkStackTo16_Int
		push	edx
		push	eax
		call	VDHCREATETHREAD
		ThunkStackTo32_Int
		pop		edx
		pop		ecx
		pop		ebx
		cmp		eax,0
		je		@F
		mov		eax,[edx]
@@:	ret
_LXA_CreateThread	endp

;*******************************************************************************
;* Start thread service threads                                                *
;*******************************************************************************
		extrn	_LX_ServiceThread : NEAR
LXA_ServiceThread_Stub:
;		mov		eax,DOS32FLATDS
;		mov		ds,eax
;		mov		es,eax
LXA_ServiceThread_Stub_restart:
		mov		eax,OFFSET FLAT:_lxa_service_thread_id
		mov		bx,ax
		shr		eax,16
		mov		edi,dword ptr _lxa_service_thread_block_time
		mov		cx,di
		shr		edi,16
		mov		dh,0
		mov		dl,4
		call	LXA_StackThunkDevHlp
		ThunkStackTo32
		cmp		eax,0
		jne		LXA_ServiceThread_Stub_errret
		call	_LX_ServiceThread
		ThunkStackTo16
LXA_ServiceThread_Stub_errret:
		cmp		dword ptr _lxa_shutdown_level,2
		jle		LXA_ServiceThread_Stub_restart
		xor		eax,eax
		mov		_lxa_service_thread_id,0
		call	VDHEXITTHREAD
		retf

		ALIGN	4
		public	_LXA_start_service_thread
_LXA_start_service_thread proc near
		mov		_lx_started_task.NTASK_eip,0
		mov		_lx_started_task.NTASK_esp,0
		mov		_lx_started_task.NTASK_prev,0
		push	edx
		mov		edx,eax
		mov		eax,offset FLAT:LXA_ServiceThread_Stub
		call	_LXA_CreateThread
		pop		edx
		ret
_LXA_start_service_thread endp


;*******************************************************************************
;*	Task stack allocation wrapper												*
;*******************************************************************************
_LXA_TaskStackAlloc	proc near
		xor		eax,eax
		LOCK
		xchg	eax,_lx_started_task.NTASK_esp
		ret
_LXA_TaskStackAlloc	endp

_LXA_TaskStackFree		proc near
		ret
_LXA_TaskStackFree		endp

;*******************************************************************************
;*	Task entry point      														*
;*******************************************************************************
		ALIGN	4
		public	_LXA_TaskStub32
_LXA_TaskStub32	proc far
		extrn	_LX_add_this_current : NEAR
;		mov		eax,DOS32FLATDS
;		mov		ds,eax
;		mov		es,eax
		ThunkTaskStackTo32
		cmp		eax,0
		jne		_LXA_TaskStub32_2
		LOCK
		xchg	eax,_lx_started_task.NTASK_eip
		push	eax
		call	_LX_add_this_current
		xor		eax,eax
		LOCK
		xchg	eax,_lx_started_task.NTASK_prev
		pop		ecx
		call	ecx
		ThunkTaskStackTo16
		xor		eax,eax
_LXA_TaskStub32_2:
		call	VDHEXITTHREAD
		retf	
_LXA_TaskStub32	endp

		ALIGN	4
		public	_LXA_DoTaskExit
_LXA_DoTaskExit	proc far
		ThunkTaskStackTo16
		call	VDHEXITTHREAD
		retf
_LXA_DoTaskExit	endp

		ALIGN	4
		extrn	_LX_kernel_starter : NEAR
		public	_LXA_kernel_starter
_LXA_kernel_starter	proc far
		ThunkStackTo32
		cmp		eax,0
		jne		@F
		call	_LX_kernel_starter
		ThunkStackTo16
@@:	xor		eax,eax
		call	VDHEXITTHREAD
		retf
_LXA_kernel_starter	endp

;******************************************************************************
; _LXA_UtilStackWaitForCaptureAndExit
; IN:	eax =	pointer to UTLStack structure
;		ebx = 	0 call VDHEXITTHREAD
;				1 retf with RPDONE bit in eax
; OUT:			doesn't return when stack is captured
;******************************************************************************
		ALIGN	4
		extrn	LXA_StackThunkDevHlp : NEAR
		extrn	_LX_process_start_error : NEAR
		public	_LXA_UtilStackWaitForCaptureAndExit
_LXA_UtilStackWaitForCaptureAndExit proc near
		push	eax
		push	ebx
		push	ecx
		push	edx
		push	edi

		mov		ecx,ebx	; VDHEXITTHREAD???
		mov		ebx,eax	; PTR to UTLStack
		ThunkTaskStackTo16

		mov		ecx,100

@@:	push	ebx
		push	ecx
		push	edx
		mov		eax,ebx
		xor		edi,edi
		mov		cx,50
		mov		dh,0
		mov		dl,4
		call	LXA_StackThunkDevHlp
		pop		edx
		pop		ecx
		pop		ebx
		test	dword ptr [ebx].UTLStack_Flags,UTLSTACK_CAPTURED
		jnz		@F
		dec		ecx
		cmp		ecx,0
		jne		@B
		je		@USW_ERR
@@:	or		dword ptr [ebx].UTLStack_Flags,UTLSTACK_CAPTUREACK
		cmp		edx,0
		jne		@F
		call	VDHEXITTHREAD
@@:	mov		eax,0100h
		retf
@USW_ERR:
		or		dword ptr [ebx].UTLStack_Flags,UTLSTACK_FREEME
		push	edx
		ThunkStackTo32
		xor		eax,eax
		mov		ax,word ptr [ebx].UTLStack_PID
		call	_LX_process_start_error
		ThunkStackTo16
		pop		edx
		mov		dword ptr FLAT:INIT_COUNT,0
		cmp		edx,0
		jne		@F
		call	VDHEXITTHREAD
@@:	mov		eax,0100h
		retf
_LXA_UtilStackWaitForCaptureAndExit endp

TEXT32 ends

DATA32	SEGMENT
		PUBLIC	_lxa_shutdown_level
		PUBLIC	_lxa_service_thread_id
		PUBLIC	_lxa_service_thread_block_time
_lxa_shutdown_level dd 0
_lxa_service_thread_id	dd 0
_lxa_service_thread_block_time	dd 10
DATA32	ends
end
