; $Id: lxheader.asm,v 1.13 2005/03/10 21:15:25 smilcke Exp $

;
; lxheader.asm
; Autor:				Stefan Milcke
; Erstellt	am:			29.03.2003
; Letzte Aenderung	am:	11.05.2004
;
		.386p
		include defcfg.inc
		include	seg32.inc

;##############################################################################
;*	DATA16
;##############################################################################
DATA16	segment

;******************************************************************************
; device header															   *
;******************************************************************************
daemon_header	dw OFFSET	DATA16:util_header	; Pointer to next device header
				dw SEG DATA16:util_header
				dw 1000100110000000b			; Device attributes
;				   ||||| +-+   ||||
;				   ||||| | |   |||+------------------ STDIN
;				   ||||| | |   ||+------------------- STDOUT
;				   ||||| | |   |+-------------------- NULL
;				   ||||| | |   +--------------------- CLOCK
;				   ||||| | |
;				   ||||| | +------------------------+ (001)	OS/2
;				   ||||| |							| (010)	DosDevIOCtl2 + SHUTDOWN
;				   ||||| +--------------------------+ (011)	Capability bit strip
;				   |||||
;				   ||||+----------------------------- OPEN/CLOSE (char)	or Removable (blk)
;				   |||+------------------------------ Sharing support
;				   ||+------------------------------- IBM
;				   |+-------------------------------- IDC entry	point
;				   +--------------------------------- char/block device	driver

				dw offset CODE16:daemon_stub_strategy	 ; Strategy	routine	entry point
				dw 0				; IDC routine entry	point
				db 'LXAPID$ '					; Device name
				db 8 dup (0)					; Reserved
				dw 0000000000010011b			; Level	3 device driver	capabilities
;							  |||||
;							  ||||+------------------ DosDevIOCtl2 + Shutdown
;							  |||+------------------- More than	16 MB support
;							  ||+-------------------- Parallel port	driver
;							  |+--------------------- Adapter device driver
;							  +---------------------- InitComplete
				dw 0000000000000000b

util_header	dw OFFSET	DATA16:help_header	; Pointer to next device header
				dw SEG DATA16:help_header
				dw 1101100110000000b			; Device attributes
;				   ||||| +-+   ||||
;				   ||||| | |   |||+------------------ STDIN
;				   ||||| | |   ||+------------------- STDOUT
;				   ||||| | |   |+-------------------- NULL
;				   ||||| | |   +--------------------- CLOCK
;				   ||||| | |
;				   ||||| | +------------------------+ (001)	OS/2
;				   ||||| |							| (010)	DosDevIOCtl2 + SHUTDOWN
;				   ||||| +--------------------------+ (011)	Capability bit strip
;				   |||||
;				   ||||+----------------------------- OPEN/CLOSE (char)	or Removable (blk)
;				   |||+------------------------------ Sharing support
;				   ||+------------------------------- IBM
;				   |+-------------------------------- IDC entry	point
;				   +--------------------------------- char/block device	driver

				dw offset CODE16:util_stub_strategy	   ; Strategy routine entry	point
				dw 0	   ; IDC routine entry point
				db 'LXAPIUT$'					; Device name
				db 8 dup (0)					; Reserved
				dw 0000000000010011b			; Level	3 device driver	capabilities
;							  |||||
;							  ||||+------------------ DosDevIOCtl2 + Shutdown
;							  |||+------------------- More than	16 MB support
;							  ||+-------------------- Parallel port	driver
;							  |+--------------------- Adapter device driver
;							  +---------------------- InitComplete
				dw 0000000000000000b

help_header	dw OFFSET DATA16:device_header	; Pointer to next device header
				dw SEG DATA16:device_header
				dw 1000100110000000b			; Device attributes
;				   ||||| +-+   ||||
;				   ||||| | |   |||+------------------ STDIN
;				   ||||| | |   ||+------------------- STDOUT
;				   ||||| | |   |+-------------------- NULL
;				   ||||| | |   +--------------------- CLOCK
;				   ||||| | |
;				   ||||| | +------------------------+ (001)	OS/2
;				   ||||| |							| (010)	DosDevIOCtl2 + SHUTDOWN
;				   ||||| +--------------------------+ (011)	Capability bit strip
;				   |||||
;				   ||||+----------------------------- OPEN/CLOSE (char)	or Removable (blk)
;				   |||+------------------------------ Sharing support
;				   ||+------------------------------- IBM
;				   |+-------------------------------- IDC entry	point
;				   +--------------------------------- char/block device	driver

				dw offset CODE16:help_stub_strategy	   ; Strategy routine entry	point
				dw 0				; IDC routine entry	point
				db 'LXAPIH$ '					; Device name
				db 8 dup (0)					; Reserved
				dw 0000000000010011b			; Level	3 device driver	capabilities
;							  |||||
;							  ||||+------------------ DosDevIOCtl2 + Shutdown
;							  |||+------------------- More than	16 MB support
;							  ||+-------------------- Parallel port	driver
;							  |+--------------------- Adapter device driver
;							  +---------------------- InitComplete
				dw 0000000000000000b

device_header	dd -1							; Pointer to next device header
				dw 1101100110000000b			; Device attributes
;				   ||||| +-+   ||||
;				   ||||| | |   |||+------------------ STDIN
;				   ||||| | |   ||+------------------- STDOUT
;				   ||||| | |   |+-------------------- NULL
;				   ||||| | |   +--------------------- CLOCK
;				   ||||| | |
;				   ||||| | +------------------------+ (001)	OS/2
;				   ||||| |							| (010)	DosDevIOCtl2 + SHUTDOWN
;				   ||||| +--------------------------+ (011)	Capability bit strip
;				   |||||
;				   ||||+----------------------------- OPEN/CLOSE (char)	or Removable (blk)
;				   |||+------------------------------ Sharing support
;				   ||+------------------------------- IBM
;				   |+-------------------------------- IDC entry	point
;				   +--------------------------------- char/block device	driver

				dw offset CODE16:device_stub_strategy  ; Strategy routine entry	point
				dw offset CODE16:device_stub_idc	   ; IDC routine entry point
				db 'LXAPI32$'					; Device name
				db 8 dup (0)					; Reserved
				dw 0000000000010011b			; Level	3 device driver	capabilities
;							  |||||
;							  ||||+------------------ DosDevIOCtl2 + Shutdown
;							  |||+------------------- More than	16 MB support
;							  ||+-------------------- Parallel port	driver
;							  |+--------------------- Adapter device driver
;							  +---------------------- InitComplete
				dw 0000000000000000b
DATA16	ends

CODE16	segment
		assume cs:CODE16, ds:DATA16
		extrn	daemon_stub_strategy : far
		extrn	help_stub_strategy : far
		extrn	device_stub_strategy : far
		extrn	device_stub_idc	: far
		extrn	util_stub_strategy : far
CODE16	ends
end

