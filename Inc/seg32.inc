; $Id: seg32.inc,v 1.18 2006/04/22 21:44:59 smilcke Exp $

; ****************
; 16Bit
; ****************
DATA16 SEGMENT WORD PUBLIC USE16 'DATA16'
DATA16 ENDS

CODE16 SEGMENT WORD PUBLIC USE16 'CODE'
CODE16 ENDS

; ****************
; 32Bit
; ****************
;_TEXT SEGMENT DWORD PUBLIC USE32 'CODE'
;_TEXT ENDS

TEXT32 SEGMENT DWORD PUBLIC USE32 'CODE'
TEXT32 ENDS

_init_text SEGMENT DWORD PUBLIC USE32 'CODE'
_init_text ENDS

_exit_text SEGMENT DWORD PUBLIC USE32 'CODE'
_exit_text ENDS

_sched_text SEGMENT DWORD PUBLIC USE32 'CODE'
_sched_text ENDS

DATA32 SEGMENT DWORD PUBLIC USE32 'DATA'
DATA32  ENDS

BSS32   SEGMENT DWORD PUBLIC USE32 'BSS'
BSS32   ENDS

c_common SEGMENT DWORD PUBLIC USE32 'DATA'
c_common ENDS

CONST32_RO SEGMENT DWORD PUBLIC USE32 'CONST'
CONST32_RO ENDS

include segdef.inc

DGROUP  group DATA32, c_common, BSS32

CGROUP  group TEXT32

;CINITGRP group _init_text

;DINITCGRP group _initcall1_init, _initcall2_init, _initcall3_init, _initcall4_init, _initcall5_init, _initcall6_init, _initcall7_init
