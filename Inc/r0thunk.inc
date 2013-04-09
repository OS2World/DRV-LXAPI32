; $Id: r0thunk.inc,v 1.8 2005/03/06 02:18:34 smilcke Exp $

; 32 bits OS/2 device driver and IFS support. Provides 32 bits kernel
; services (DevHelp) and utility functions to 32 bits OS/2 ring 0 code
; (device drivers and installable file system drivers).
; Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

extrn DOS32FLATCS : abs                ; ring 0 FLAT kernel data selector
extrn DOS32FLATDS : abs                ; ring 0 FLAT kernel data selector

DATA32 SEGMENT
;        extrn DevHelp2  : dword
ifdef FSD32
;	extrn _TKSSBase : dword
endif
DATA32 ends

ifdef FSD32
;%out  FSD32 defined !
jmp32 macro target
        db 066h
        db 0eah
        dd offset FLAT:target
        dw Dos32FlatCS
endm
endif
ifdef MWDD32
;%out MWDD32 defined !
jmp32 macro target
        jmp far ptr FLAT:target
endm
endif

ifdef FSD32
;%out  FSD32 defined !
call32 macro target
        db 066h
        db 09ah
        dd offset FLAT:target
        dw Dos32FlatCS
endm
endif
ifdef MWDD32
;%out  MWDD32 defined !
call32 macro target
        call far ptr FLAT:target
endm
endif

;The following macro is used to execute a far call when in protected
;mode. You must supply the desired offset and selector. It also
;assumes 32-bit mode because we want the DD to give a 32-bit offset.
;If using this in 16-bit mode, you must change the DD to DW.
;This can be used for task switches or calling gates.
PCALLF32_32 MACRO SEL1,OFF1
   DB 9AH
   DD OFF1
   DW SEL1
ENDM
;This macro is for making a far call from a 32-bit segment to a
;16-bit segment. The 66H tells the CPU to use a 16-bit operand
;instead of a 32-bit. Assumes OFF1 is defined in a USE16 segment.
PCALLF32_16 MACRO SEL1,OFF1
   DB 66H
   DB 09AH
   DW OFF1
   DW SEL1
ENDM
;This macro is for making a far jump from a 32-bit segment to a
;16-bit segment. The 66H tells the CPU to use a 16-bit operand
;instead of a 32-bit. Assumes OFF1 is defined in a USE16 segment.
PJMPF32_16 MACRO SEL1,OFF1
   DB 66H
   DB 0EAH
   DW OFF1
   DW SEL1
ENDM
;This macro is for making a far jump from a 16-bit segment to a
;32-bit segment. The 66H tells the CPU to use a 32-bit operand
;instead of a 16-bit. Assumes OFF1 is defined in a USE32 segment.
PJMPF16_32 MACRO SEL1,OFF1
   DB 66H
   DB 0EAH
   DD OFFSET OFF1
   DW SEL1
ENDM
;This macro is for making a far jump from a 32-bit segment to a
;32-bit segment. This can be used for task switches. Assumes OFF1 is
;defined in a 32-bit segment.
PJMPF32_32 MACRO SEL1,OFF1
   DB 0EAH
   DD OFF1
   DW SEL1
ENDM
;This macro is for making a far jump from a 16-bit segment to a
;16-bit segment. This can be used for task switches. Assumes OFF1 is
;defined in a 16-bit segment.
PJMPF16_16 MACRO SEL1,OFF1
   DB 0EAH
   DW OFF1
   DW SEL1
ENDM
