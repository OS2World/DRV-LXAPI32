; $Id: lxbases.inc,v 1.8 2005/03/15 23:37:54 smilcke Exp $

;
; lxbases.inc
; Autor:               Stefan Milcke
; Erstellt am:         20.02.2003
; Letzte Aenderung am: 14.03.2005
;

PDDSTACK_FREE		equ 0
PDDSTACK_USED		equ 1

PDDSTACK_NOTASK		equ 0
PDDSTACK_ISTASK		equ 2

UTLSTACK_FREE			equ 0
UTLSTACK_USED			equ 1
UTLSTACK_CAPTURED		equ 4
UTLSTACK_CAPTUREACK	equ 8
UTLSTACK_FREEME		equ 16

; PDDStack structure
PDDStack struc
PDDStack_Next		dd ?
PDDStack_Base		dd ?
PDDStack_Flags		dd ?
PDDStack_PID		dw ?
PDDStack_TID		dw ?
PDDStack ends

; UTLStack structure
UTLStack struc
UTLStack_Next		dd ?
UTLStack_Base		dd ?
UTLStack_Flags		dd ?
UTLStack_PID		dw ?
UTLStack_TID		dw ?
UTLStack ends

; NTASK structure for start a new task
NTASK struc
NTASK_esp			dd ?
NTASK_eip			dd ?
NTASK_prev			dd ?
NTASK ends

;
; lx_dosopen_struct
lxdoso		struc
lxdosoFileName	    dd ?
lxdosoFileHandle	dd ?
lxdosoActionTaken	dd ?
lxdosoFileSize		dd ?
lxdosoFileAttr		dd ?
lxdosoOpenFlag		dd ?
lxdosoOpenMode		dd ?
lxdosoEABuf		dd ?
lxdoso		ends
