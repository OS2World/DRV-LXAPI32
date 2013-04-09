; $Id: lxrmcall.asm,v 1.14 2005/03/06 02:18:29 smilcke Exp $

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
		include	r0thunk.inc
		include	rp.inc
		include	rm.inc
		include	devhlp32.inc
		include	infoseg.inc
		include	lxbases.inc
		include	lxbasem.inc

;##############################################################################
;*	Resource Manager
;##############################################################################
TEXT32 SEGMENT
ASSUME	CS:FLAT, DS:FLAT, ES:FLAT
		extrn	LXA_FixSelDPL		: NEAR
		extrn	LXA_RestoreSelDPL	: NEAR
TEXT32	ends

DATA16	segment
		extrn DevHelpFnPtr : dword
DATA16	ends

DATA16	segment
		public	RM_Help0
		public	RM_Help1
		public	RM_Help3
		public	RMFlags
		public	RMIDCTable
		public	RMNAME
		public	rm_hDriver
		public	rm_hAdapter
		public	rm_hDevice
		public	rm_hResource
		public	rm_tmpStr1
		public	rm_tmpStr2
		public	rm_tmpStr3
		public	rm_DriverStruct
		public	rm_AdapterStruct
		public	rm_DeviceStruct
RM_Help0		DWORD 0
RM_Help1		DWORD 0
RM_Help3		DWORD 0
RMFlags		DWORD 0
RMIDCTable	BYTE[12] dup (0)
RMNAME		BYTE "RESMGR$ ",0
rm_hDriver		dword	0
rm_hAdapter	dword	0
rm_hDevice		dword	0
rm_hResource	dword	0
rm_ModifyAction dword	0
rm_tmpStr1		BYTE[255] dup (0)
rm_ResourceList label byte
rm_tmpStr2		BYTE[255] dup (0)
rm_tmpStr3		BYTE[255] dup (0)
rm_DriverStruct RM_DRIVERSTRUCT <DATA16:rm_tmpStr1,DATA16:rm_tmpStr2,DATA16:rm_tmpStr3>
rm_AdapterStruct RM_ADAPTERSTRUCT <DATA16:rm_tmpStr1>
rm_DeviceStruct RM_DEVICESTRUCT <DATA16:rm_tmpStr1>
;rm_DriverStruct RM_DRIVERSTRUCT <>
;rm_AdapterStruct RM_ADAPTERSTRUCT	<>
;rm_DeviceStruct RM_DEVICESTRUCT <>
rm_ResourceStruct DWORD 0		; Resource type
				BYTE	12 DUP (0)
				DWORD	0		; Reserved
rm_AdjunctBuffer BYTE 1024	DUP	(0)
DATA16	ends

DATA32 SEGMENT
		public	_rm32_hDriver
		public	_rm32_hAdapter
		public	_rm32_hDevice
		public	_rm32_hResource
		public	_rm32_ModifyAction
		public	_rm32_DriverStruct
		public	_rm32_AdapterStruct
		public	_rm32_DeviceStruct
		public	_rm32_ResourceStruct
		public	_rm32_AdjunctBuffer
		public	_rm32_ResourceList
		public	_rm32_tmpStr1
		public	_rm32_tmpStr2
		public	_rm32_tmpStr3
		extrn  tempeax			: dword
		extrn  tempedx			: dword
		extrn  tempesi			: dword
		extrn  cpuflags			: dword
		extrn  fInitStack		: dword
		extrn  intSwitchStack	: dword
		extrn  SMP_Lock			: dword
_rm32_hDriver	dw OFFSET DATA16:rm_hDriver
				dw SEG DATA16:rm_hDriver
_rm32_hAdapter	dw OFFSET DATA16:rm_hAdapter
				dw SEG DATA16:rm_hAdapter
_rm32_hDevice	dw OFFSET DATA16:rm_hDevice
				dw SEG DATA16:rm_hDevice
_rm32_hResource dw	OFFSET DATA16:rm_hResource
				dw SEG DATA16:rm_hResource
_rm32_ModifyAction	dw OFFSET DATA16:rm_ModifyAction
				dw SEG DATA16:rm_ModifyAction
_rm32_DriverStruct	dw OFFSET DATA16:rm_DriverStruct
				dw SEG DATA16:rm_DriverStruct
_rm32_AdapterStruct dw	OFFSET DATA16:rm_AdapterStruct
				dw SEG DATA16:rm_AdapterStruct
_rm32_DeviceStruct	dw OFFSET DATA16:rm_DeviceStruct
				dw SEG DATA16:rm_DeviceStruct
_rm32_ResourceStruct dw OFFSET	DATA16:rm_ResourceStruct
				dw SEG DATA16:rm_ResourceStruct
_rm32_AdjunctBuffer dw	OFFSET DATA16:rm_AdjunctBuffer
				dw SEG DATA16:rm_AdjunctBuffer
_rm32_ResourceList	dw OFFSET DATA16:rm_ResourceList
				dw SEG DATA16:rm_ResourceList
_rm32_tmpStr1	dw OFFSET DATA16:rm_tmpStr1
				dw SEG DATA16:rm_tmpStr1
_rm32_tmpStr2	dw OFFSET DATA16:rm_tmpStr2
				dw SEG DATA16:rm_tmpStr2
_rm32_tmpStr3	dw OFFSET DATA16:rm_tmpStr3
				dw SEG DATA16:rm_tmpStr3

DATA32	ends

CODE16	segment
		assume cs:CODE16,ds:DATA16
;---------------------------------	RM_ATTACHDD	---------------------------------
		ALIGN	2
		public	RM_ATTACHDD
RM_ATTACHDD	proc near
		enter	0,0
		push	di
		mov		bx,06h[bp]
		mov		di,04h[bp]
		mov		dl,2ah
		call	dword ptr DevHelpFnPtr
		jb		short @RMATT1
		sub		ax,ax
@RMATT1:
		pop		di
		leave
		ret		4
		ALIGN	2
RM_ATTACHDD	endp

;------------------------------------ _GetCS -----------------------------------
		ALIGN	2
_GetCS	proc	near
		enter	2,0
		push	cs
		pop		word ptr -02h[bp]
		mov		ax,-02h[bp]
		leave
		ret
_GetCS	endp

;----------------------------------- CallRM -----------------------------------
		ALIGN	2
		public	CallRM
CallRM proc far
		enter	2,0
		call	_GetCS
		test	al,03h
		je		short @CALLRM1
		push	word ptr 0ah[bp]
		push	word ptr 08h[bp]
		push	word ptr 06h[bp]
		call	dword ptr RM_Help3
		add		sp,6
		leave
		retf
		ALIGN	2
@CALLRM1:
		push	word ptr 06h[bp]
		call	dword ptr RM_Help0
		leave
		retf
CallRM endp

;---------------------------------	__GetAdjList --------------------------------
; Get pointer to adjunct list in ax:si
		ALIGN	2
__GetAdjList proc near
		mov		si,offset rm_AdjunctBuffer
		push	si
@@:	mov		ax,ds:[si]
		or		ax,ds:[si+2]
		je		@F
		mov		ax,ds:[si+4]
		add		ax,si
		mov		ds:[si],ax
		mov		ds:[si+2],ds
		mov		si,ax
		jmp		short @B
@@:	pop		si
		cmp		word ptr ds:[si+4],0
		je		@F
		mov		ax,ds
		ret
@@:	mov		ax,0
		mov		si,0
		ret
__GetAdjList endp
CODE16	ends

;******************************************************************************
; RMCreateDriver
;******************************************************************************
CODE16	segment
		assume cs:CODE16,ds:DATA16
		public	RMCreateDriver
		public	th3216_rmcreatedriver
		ALIGN	2
RMCreateDriver proc far
		enter	4,0
		test	byte ptr RMFlags,1
		je		short @RMCDRVR4
@RMCDRVR3:
		lea		ax,06h[bp]
		push	ss
		push	ax
		push	02h
		call	CallRM
		add		sp,06h
		leave
		retf
		ALIGN	2
@RMCDRVR4:
		test	byte ptr RMFlags,2
		je		short @RMCDRVR6
@RMCDRVR5:
		les		bx,0ah[bp]
		mov		word ptr es:[bx],0ffffh
		mov		word ptr es:02h[bx],0ffffh
		sub		ax,ax
		leave
		retf
		ALIGN	2
@RMCDRVR6:
		mov		ax,word	ptr	(DevHelpFnPtr +	02h)
		or		ax,word	ptr	(DevHelpFnPtr)
		jne		short @RMCDRVR7
		mov		ax,8
		leave
		retf
		ALIGN	2
@RMCDRVR7:
		push	offset RMNAME
		push	offset RMIDCTable
		call	RM_ATTACHDD
		or		ax,ax
		je		short @RMCDRVR8
		or		byte ptr RMFlags,2
		jmp		@RMCDRVR5
		ALIGN	2
@RMCDRVR8:
		mov		ax,word	ptr	RMIDCTable	+ 04h
		mov		word ptr RM_Help3 + 02h,ax
		mov		word ptr RM_Help3,0
		mov		ax,word	ptr	RMIDCTable	+ 02h
		mov		word ptr RM_Help0 + 02h,ax
		mov		ax,word	ptr	RMIDCTable	+06h
		mov		word ptr RM_Help0,ax
		or		byte ptr RMFlags,05h
		jmp		short @RMCDRVR3
RMCreateDriver endp
		ALIGN	2
th3216_rmcreatedriver proc	far
		TH3216Enter
		push	ds
		push	offset rm_hDriver
		push	ds
		push	offset rm_DriverStruct
		call	RMCreateDriver
		add		sp,8
		TH3216Return
th3216_rmcreatedriver endp

CODE16	ends

RM32Thunk	_RM32CreateDriverASM,th3216_rmcreatedriver,th3216_rmcreatedriver_ptr

;******************************************************************************
; RMDestroyDriver
;******************************************************************************
CODE16	segment
		assume	cs:CODE16,ds:DATA16
		public	RMDestroyDriver
		public	th3216_rmdestroydriver
		ALIGN	2
RMDestroyDriver proc far
		push	bp
		mov		bp,sp
		test	byte ptr RMFlags,1
		je		@RMDDRVR1
		lea		ax,06[bp]
		push	ss
		push	ax
		push	03h
		call	CallRM
		mov		sp,bp
		leave
		retf
		ALIGN	2
@RMDDRVR1:
		test	byte ptr RMFlags,2
		je		@RMDDRVR2
		sub		ax,ax
		leave
		retf
		ALIGN	2
@RMDDRVR2:
		mov		ax,1
		leave
		retf
RMDestroyDriver endp
		ALIGN	2

th3216_rmdestroydriver	proc far
		TH3216Enter
		push	dword ptr rm_hDriver
		call	RMDestroyDriver
		add		sp,4
		TH3216Return
th3216_rmdestroydriver	endp
CODE16	ends

RM32Thunk	_RM32DestroyDriverASM,th3216_rmdestroydriver,th3216_rmdestroydriver_ptr

;******************************************************************************
; RMCreateAdapter
;******************************************************************************
CODE16	segment
		assume cs:CODE16,ds:DATA16
		public	RMCreateAdapter
		public	th3216_rmcreateadapter
		ALIGN	2
RMCreateAdapter proc far
		push	bp
		mov		bp,sp
		test	byte ptr RMFlags,1
		je		short @RMCADPT1
		lea		ax,06h[bp]
		push	ss
		push	ax
		push	04h
		call	CallRM
		mov		sp,bp
		leave
		retf
		ALIGN	2
@RMCADPT1:
		test	byte ptr RMFlags,2
		je		short @RMCADPT2
		les		bx,0ah[bp]
		mov		word ptr es:[bx],0ffffh
		mov		word ptr es:02h[bx],0ffffh
		sub		ax,ax
		leave
		retf
		ALIGN	2
@RMCADPT2:
		mov		ax,1
		leave
		retf
RMCreateAdapter endp
		ALIGN	2
th3216_rmcreateadapter	proc far
		TH3216Enter
		call	__GetAdjList
		mov		word ptr rm_AdapterStruct.rm_AD_pAdjunctList,si
		mov		word ptr rm_AdapterStruct.rm_AD_pAdjunctList+2,ax

		mov		ax,0
		mov		bx,0
		cmp		dword ptr rm_ResourceList,0
		je		@F
		mov		ax,DATA16
		mov		bx,offset rm_ResourceList
@@:	push	ax
		push	bx
		push	dword ptr rm_hDevice
		push	ds
		push	offset rm_AdapterStruct
		push	ds
		push	offset rm_hAdapter
		push	dword ptr rm_hDriver
		call	RMCreateAdapter
		add		sp,20
		TH3216Return
th3216_rmcreateadapter	endp
CODE16	ends

RM32Thunk	_RM32CreateAdapterASM,th3216_rmcreateadapter,th3216_rmcreateadapter_ptr

;******************************************************************************
; RMDestroyAdapter
;******************************************************************************
CODE16	segment
		assume	cs:CODE16,ds:DATA16
		public	RMDestroyAdapter
		public	th3216_rmdestroyadapter
		ALIGN	2
RMDestroyAdapter proc	far
		push	bp
		mov		bp,sp
		test	byte ptr RMFlags,1
		je		@RMDADPT1
		lea		ax,06[bp]
		push	ss
		push	ax
		push	05h
		call	CallRM
		mov		sp,bp
		leave
		retf
		ALIGN	2
@RMDADPT1:
		test	byte ptr RMFlags,2
		je		@RMDADPT2
		sub		ax,ax
		leave
		retf
		nop
@RMDADPT2:
		mov		ax,1
		leave
		retf
RMDestroyAdapter endp
		ALIGN	2
th3216_rmdestroyadapter proc far
		TH3216Enter
		push	dword ptr rm_hAdapter
		push	dword ptr rm_hDriver
		call	RMDestroyAdapter
		add		sp,8
		TH3216Return
th3216_rmdestroyadapter endp
CODE16	ends

RM32Thunk	_RM32DestroyAdapterASM,th3216_rmdestroyadapter,th3216_rmdestroyadapter_ptr

;******************************************************************************
; RMCreateDevice
;******************************************************************************
CODE16	segment
		assume cs:CODE16,ds:DATA16
		public	RMCreateDevice
		public	th3216_rmcreatedevice
		ALIGN	2
RMCreateDevice proc far
		push	bp
		mov		bp,sp
		test	byte ptr RMFlags,1
		je		short @RMCDEV1
		lea		ax,06h[bp]
		push	ss
		push	ax
		push	06h
		call	CallRM
		mov		sp,bp
		leave
		retf
		ALIGN	2
@RMCDEV1:
		test	byte ptr RMFlags,2
		je		short @RMCDEV2
		les		bx,0ah[bp]
		mov		word ptr es:[bx],0ffffh
		mov		word ptr es:02h[bx],0ffffh
		sub		ax,ax
		leave
		retf
		ALIGN	2
@RMCDEV2:
		mov		ax,1
		leave
		retf
RMCreateDevice endp
		ALIGN	2
th3216_rmcreatedevice proc	far
		TH3216Enter
		call	__GetAdjList
		mov		word ptr rm_DeviceStruct.rm_DE_pAdjunctList,si
		mov		word ptr rm_DeviceStruct.rm_DE_pAdjunctList+2,ax

		mov		ax,0
		mov		bx,0
		cmp		dword ptr rm_ResourceList,0
		je		@F
		mov		ax,DATA16
		mov		bx,offset rm_ResourceList
@@:	push	ax
		push	bx
		push	dword ptr rm_hAdapter
		push	ds
		push	offset rm_DeviceStruct
		push	ds
		push	offset rm_hDevice
		push	dword ptr rm_hDriver
		call	RMCreateDevice
		add		sp,20
		TH3216Return
th3216_rmcreatedevice	endp
CODE16	ends

RM32Thunk	_RM32CreateDeviceASM,th3216_rmcreatedevice,th3216_rmcreatedevice_ptr

;******************************************************************************
; RMDestroyDevice
;******************************************************************************
CODE16	segment
		assume	cs:CODE16,ds:DATA16
		public	RMDestroyDevice
		public	th3216_rmdestroydevice
		ALIGN	2
RMDestroyDevice proc far
		push	bp
		mov		bp,sp
		test	byte ptr RMFlags,1
		je		@RMDDEV1
		lea		ax,06h[bp]
		push	ss
		push	ax
		push	07h
		call	CallRM
		mov		sp,bp
		leave
		retf
		ALIGN	2
@RMDDEV1:
		test	byte ptr RMFlags,2
		je		@RMDDEV2
		sub		ax,ax
		leave
		retf
		ALIGN	2
@RMDDEV2:
		mov		ax,1
		leave
		retf
RMDestroyDevice endp
		ALIGN	2
th3216_rmdestroydevice	proc far
		TH3216Enter
		push	dword ptr rm_hDevice
		push	dword ptr rm_hDriver
		call	RMDestroyDevice
		add		sp,8
		TH3216Return
th3216_rmdestroydevice	endp
CODE16	ends

RM32Thunk	_RM32DestroyDeviceASM,th3216_rmdestroydevice,th3216_rmdestroydevice_ptr

;******************************************************************************
; RMAllocResource
;******************************************************************************
CODE16	segment
		assume	cs:CODE16,ds:DATA16
		public	RMAllocResource
		public	th3216_rmallocresource
		ALIGN	2
RMAllocResource proc far
		push	bp
		mov		bp,sp
		test	byte ptr RMFlags,1
		je		@RMARES1
		lea		ax,06h[bp]
		push	ss
		push	ax
		push	08h
		call	CallRM
		mov		sp,bp
		leave
		retf
		ALIGN	2
@RMARES1:
		test	byte ptr RMFlags,2
		je		@RMARES2
		les		bx,0ah[bp]
		mov		word ptr es:[bx],0ffffh
		mov		word ptr es:02h[bx],0ffffh
		sub		ax,ax
		leave
		retf
		ALIGN	2
@RMARES2:
		mov		ax,1
		leave
		retf
RMAllocResource endp
		ALIGN	2
th3216_rmallocresource	proc far
		TH3216Enter
		push	ds
		push	offset rm_ResourceStruct
		push	ds
		push	offset rm_hResource
		push	dword ptr rm_hDriver
		call	RMAllocResource
		add		sp,12
		TH3216Return
th3216_rmallocresource	endp
CODE16	ends

RM32Thunk	_RM32AllocResourceASM,th3216_rmallocresource,th3216_rmallocresource_ptr

;******************************************************************************
; RMDeallocResource
;******************************************************************************
CODE16	segment
		assume	cs:CODE16,ds:DATA16
		public	RMDeallocResource
		public	th3216_rmdeallocresource
		ALIGN	2
RMDeallocResource	proc far
		push	bp
		mov		bp,sp
		test	byte ptr RMFlags,1
		je		@RMDRES1
		lea		ax,06h[bp]
		push	ss
		push	ax
		push	09h
		call	CallRM
		mov		sp,bp
		leave
		retf
		ALIGN	2
@RMDRES1:
		test	byte ptr RMFlags,2
		je		@RMDRES2
		sub		ax,ax
		leave
		retf
		ALIGN	2
@RMDRES2:
		mov		ax,1
		leave
		retf
RMDeallocResource	endp
		ALIGN	2
th3216_rmdeallocresource proc far
		TH3216Enter
		push	dword ptr rm_hResource
		push	dword ptr rm_hDriver
		call	RMDeallocResource
		add		sp,8
		TH3216Return
th3216_rmdeallocresource endp
CODE16	ends

RM32Thunk _RM32DeallocResourceASM,th3216_rmdeallocresource,th3216_rmdeallocresource_ptr

;******************************************************************************
; RMCreateLDev
;******************************************************************************

;******************************************************************************
; RMDestroyLDev
;******************************************************************************

;******************************************************************************
; RMCreateSysName
;******************************************************************************

;******************************************************************************
; RMDestroySysName
;******************************************************************************

;******************************************************************************
; RMADDToHDEVICE
;******************************************************************************

;******************************************************************************
; RMKeyToHandleList
;******************************************************************************

;******************************************************************************
; RMHandleToType
;******************************************************************************

;******************************************************************************
; RMHandleToParent
;******************************************************************************

;******************************************************************************
; RMModifyResources
;******************************************************************************
CODE16	segment
		assume	cs:CODE16,ds:DATA16
		public	RMModifyResources
		public	th3216_rmmodifyresources
		ALIGN	2
RMModifyResources	proc far
		push	bp
		mov		bp,sp
		test	byte ptr RMFlags,1
		je		@RMMRES1
		lea		ax,06h[bp]
		push	ss
		push	ax
		push	1bh
;		 push	 cs
		call	CallRM
		mov		sp,bp
		leave
		retf
		ALIGN	2
@RMMRES1:
		test	byte ptr RMFlags,2
		je		@RMMRES2
		mov		ax,15h
		leave
		retf
		ALIGN	2
@RMMRES2:
		mov		ax,1
		leave
		retf
RMModifyResources	endp
		ALIGN	2
th3216_rmmodifyresources proc far
		TH3216Enter
		push	dword ptr rm_hResource
		push	word ptr rm_ModifyAction
		push	dword ptr rm_hAdapter
		push	dword ptr rm_hDriver
		call	RMModifyResources
		add		sp,14
		TH3216Return
th3216_rmmodifyresources endp
CODE16	ends

RM32Thunk _RM32ModifyResourcesASM,th3216_rmmodifyresources,th3216_rmmodifyresources_ptr

;******************************************************************************
; RMParseScsiInquiry
;******************************************************************************

;******************************************************************************
; RMUpdateAdjunct
;******************************************************************************

;******************************************************************************
; RMAdjToHandleList
;******************************************************************************

;******************************************************************************
; RMHDevToHLDev
;******************************************************************************

;******************************************************************************
; RMResToHandleList
;******************************************************************************

;******************************************************************************
; RMGetNodeInfo
;******************************************************************************

;******************************************************************************
; RMCreateDetected
;******************************************************************************

;******************************************************************************
; RMDestroyDetected
;******************************************************************************

;******************************************************************************
; RMDevIDToHandleList
;******************************************************************************

;******************************************************************************
; RMHandleToResourceHandleList
;******************************************************************************

;******************************************************************************
; RMModifyNodeFlags
;******************************************************************************

;******************************************************************************
; RMConvertID
;******************************************************************************

;******************************************************************************
; RMGetCommandLine
;******************************************************************************

;******************************************************************************
; RMGetVersion
;******************************************************************************

;******************************************************************************
; RMSetSnoopLevel
;******************************************************************************

;******************************************************************************
; RMSaveDetectedData
;******************************************************************************

;******************************************************************************
; RMDeleteDetectedData
;******************************************************************************
end

