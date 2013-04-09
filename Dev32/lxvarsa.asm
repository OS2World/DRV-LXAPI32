; $Id: lxvarsa.asm,v 1.2 2005/03/06 02:18:33 smilcke Exp $

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

DATA32 SEGMENT

	PUBLIC	tempeax
	tempeax	dd 0

	PUBLIC	tempedx
	tempedx	dd 0

	PUBLIC	tempesi
	tempesi	dd 0

	PUBLIC	cpuflags
	cpuflags	dd 0

	PUBLIC	fInitStack
	fInitStack	dd 0

	PUBLIC	SMP_Lock
	SMP_Lock	dd 0

DATA32	ends

end

