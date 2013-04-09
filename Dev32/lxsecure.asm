; $Id: lxsecure.asm,v 1.8 2005/07/16 22:44:16 smilcke Exp $

;
; lxsecure.asm
; Autor:				Stefan Milcke
; Erstellt	am:			04.10.2004
; Letzte Aenderung	am:	04.10.2004
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

;##############################################################################
;*	Constants
;##############################################################################

;##############################################################################
;*	DATA32
;##############################################################################
DATA32 SEGMENT
		extrn	intSwitchStack : dword
SecHlpVersionMajor	dw 0
SecHlpVersionMinor	dw 0
SecHlpRead			dd 0
SecHlpWrite		dd 0
SecHlpOpen			dd 0
SecHlpClose		dd 0
SecHlpQFileSize	dd 0
SecHlpChgFilePtr	dd 0
SecHlpSFFromSFN	dd 0
SecHlpFindNext		dd 0
SecHlpPathFromSFN	dd 0
SecHlpApDemSVC		dd 0

SecHlpPushDWORDS	dd 5,5,4,1,2,4,1,1,1,255,255

DATA32	ENDS

;##############################################################################
;*	CODE16
;##############################################################################
CODE16	SEGMENT
		ASSUME cs:CODE16, ds:DATA16
		extrn DevHelpFnPtr : dword
CODE16	ENDS

;##############################################################################
;*	TEXT32
;##############################################################################
TEXT32 SEGMENT
		ASSUME	CS:FLAT, DS:FLAT, ES:FLAT
		extrn	DevHlp : near
		extrn	LXA_FixSelDPL : near
		extrn	LXA_RestoreSelDPL : near
TEXT32	ENDS

;##############################################################################
;*	Helper routine to get secure help entry points
;##############################################################################
TEXT32 SEGMENT
SecGetSecPtr	proc near
		pushad
		mov		eax,048a78df8h
		mov		ecx,offset FLAT:SecHlpVersionMajor
		mov		dl,044h
		call	DevHlp
		popad
		ret
SecGetSecPtr	endp
TEXT32	ENDS

;##############################################################################
;*	Security calls
;##############################################################################
SECHLPCALL MACRO fn,n
fn		proc near
		public	fn
       mov		al,n
		jmp		short __SecHlpCall
fn		endp
ENDM

TEXT32 SEGMENT
		ALIGN	4
SECHLPCALL _SecHlpRead,0
SECHLPCALL _SecHlpWrite,1
SECHLPCALL __SecHlpOpen,2
SECHLPCALL _SecHlpClose,3
SECHLPCALL _SecHlpQFileSize,4
SECHLPCALL _SecHlpChgFilePtr,5
SECHLPCALL _SecHlpSFFromSFN,6
SECHLPCALL _SecHlpFindNext,7
SECHLPCALL _SecHlpPathFromSFN,8
TEXT32	ENDS

;##############################################################################
;*	Helper routine to call a secure helper function
;* EAX = Routine to call
;##############################################################################
TEXT32 SEGMENT
		ALIGN	4
__SecHlpCall	proc near
		and		eax,0Fh
		cmp		dword ptr [FLAT:SecHlpRead],0
		jne		@F
		call	SecGetSecPtr
@@:	push	ebp
		mov		ebp,esp
		push	ebx
		push	eax
		push	ecx
		push	edx
		push	esi
		push	edi
		push	ds
		push	es
		mov		ebx,eax
		ThunkStackTo16_Int
		mov		ecx,dword ptr [FLAT:SecHlpPushDWORDS+ebx*4]	; Number of dwords to push
@@:	mov		eax,dword ptr FLAT:[ecx*4+ebp+4]			; Get from 32Bit stack
		push	eax											; and push
		dec		ecx
		jnz		@B
		mov		eax,dword ptr [FLAT:SecHlpRead+ebx*4]		; Get routine entry point
		call	eax
		and		eax,0FFFFh									; Mask return code
		mov		ebx,ebp
		mov		ebx,[ebx-08]	; Get routine number (Saved in push sequence above)
		mov		ebx,dword ptr [FLAT:SecHlpPushDWORDS+ebx*4]	; Number of dwords pushed
		shl		ebx,2										; *4
		add		esp,ebx										; Adjust stackpointer
		ThunkStackTo32_Int
		pop		es
		pop		ds
		pop		edi
		pop		esi
		pop		edx
		pop		ecx
		add		esp,4	; Skip saved EAX
		pop		ebx
		pop		ebp
		ret
__SecHlpCall	endp
TEXT32	ENDS

;##############################################################################
;*	Copy file name to stack because it may be in code segment
;##############################################################################
TEXT32 SEGMENT
		PUBLIC	_SecHlpOpen
_SecHlpOpen	proc near
		; ebp+ 8 = file name
		; ebp+12 = pSfn
		; ebp+16 = openFlags
		; ebp+20 = openMode
		; ebp-300= new file name
		enter	300,0
		push	edi
		push	esi
		push	ebx
		push	ecx

		mov		esi,[ebp+ 8] ; file name
		lea		edi,[ebp-300]; new file name
		mov		ecx,300
@@:	dec		ecx
		js		@F
		lodsb
		stosb
		test	al,al
		jne		@B
		stosb
@@:
		mov		eax,[ebp+20]
		push	eax
		mov		eax,[ebp+16]
		push	eax
		mov		eax,[ebp+12]
		push	eax
		lea		eax,[ebp-300]
		push	eax
		call	__SecHlpOpen
		add		esp,16

		pop		ecx
		pop		ebx
		pop		esi
		pop		edi
		leave
		ret
_SecHlpOpen	endp		
TEXT32	ENDS

END
