; $Id: lxend.asm,v 1.27 2006/04/27 21:17:51 smilcke Exp $

;
; lxend.asm for LXAPI32 device driver
; Autor:               Stefan Milcke
; Erstellt am:         26.11.2001
; Letzte Aenderung am: 26.04.2006

	.386p
		include defcfg.inc
       include seg32.inc
       include lxbasem.inc

TEXT32 SEGMENT
		PUBLIC __etext
__etext LABEL BYTE
TEXT32 ENDS

DATA32 SEGMENT
		PUBLIC	_pg0
_pg0	LABEL BYTE
		PUBLIC __edata
__edata LABEL BYTE
DATA32 ENDS

;********************
;* Define END Markers
;********************
SEGENDMARKER DATA16
SEGENDMARKER CODE16
SEGENDMARKER TEXT32
SEGENDMARKER DATA32
SEGENDMARKER BSS32
SEGENDMARKER CONST32_RO

include segend.inc

IFDEF TARGET_OS2_ELF
SEGMARKER _init_setup,___setup_end
SEGMARKER __ksymtab,___stop___ksymtab
SEGMARKER __ksymtab_gpl,___stop___ksymtab_gpl
ENDIF

end
