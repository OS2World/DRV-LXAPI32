; $Id: rm.inc,v 1.8 2005/03/06 02:18:34 smilcke Exp $

;
; rm.inc
; Autor:               Stefan Milcke
; Erstellt am:         20.02.2002
; Letzte Aenderung am: 09.04.2003
;
RM_DRIVERSTRUCT struc
       rm_DR_DrvName           DWORD ?
       rm_DR_DrvDescript       DWORD ?
       rm_DR_VendorName        DWORD ?
       rm_DR_MajorVer          BYTE 0
       rm_DR_MinorVer          BYTE 0
       rm_DR_DateYear          WORD 0
       rm_DR_DateMonth         BYTE 0
       rm_DR_DateDay           BYTE 0
       rm_DR_DrvrFlags         WORD 0
       rm_DR_DrvrType          WORD 0
       rm_DR_DrvrSubType       WORD 0
       rm_DR_DrvrCallback      DWORD 0
RM_DRIVERSTRUCT ends

RM_ADAPTERSTRUCT struc
       rm_AD_AdaptDescriptName DWORD ?
       rm_AD_AdaptFlags        WORD 0
       rm_AD_BaseType          WORD 0
       rm_AD_SubType           WORD 0
       rm_AD_InterfaceType     WORD 0
       rm_AD_HostBusType       WORD 0
       rm_AD_HostBusWidth      WORD 0
       rm_AD_pAdjunctList      DWORD 0
       rm_AD_Reserved          DWORD 0
RM_ADAPTERSTRUCT ends

RM_DEVICESTRUCT struc
       rm_DE_DevDescriptName   DWORD ?
       rm_DE_DevFlags          WORD 0
       rm_DE_DevType           WORD 0
       rm_DE_pAdjunctList      DWORD 0
RM_DEVICESTRUCT ends

RM_ADJUNCT struc
       rm_ADJ_pNextAdj         DWORD 0
       rm_ADJ_AdjLength        WORD 0
       rm_ADJ_AdjType          WORD 0
RM_ADJUNCT ends

; Macros
RM32Enter MACRO
       push    ebp
       ThunkStackTo16_Int
       xor     eax,eax
ENDM

RM32Return MACRO
       ThunkStackTo32_Int
@@:    pop     ebp
       ret
ENDM

RM32Thunk MACRO procname,thunkname,thunkptr
DATA32 SEGMENT
       public  thunkptr
       thunkptr    dd OFFSET CODE16:thunkname
                   dw SEG CODE16:thunkname
                   dw 0
DATA32 ends
TEXT32 SEGMENT
       assume  cs:FLAT, ds:FLAT, es:FLAT
       ALIGN 4
       public  procname
procname   proc near
       RM32Enter
       call    fword ptr thunkptr
       RM32Return
procname   endp
TEXT32 ends

ENDM
