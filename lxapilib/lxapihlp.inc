; $Id: lxapihlp.inc,v 1.10 2004/07/19 22:37:15 smilcke Exp $

;
; lxapi.inc
; Autor:               Stefan Milcke
; Erstellt am:         08.12.2001
; Letzte Aenderung am: 22.07.2003
;

TEXT32 SEGMENT
extrn init_lxapi_asm32     : far
extrn thunk3216_attachdd   : far
extrn thunk3216_callidc    : far
extrn _AttachToDDASM       : near
extrn _CallIDC             : near
TEXT32 ENDS

DATA32 SEGMENT
extrn LX_THUNKAPI          : dword
extrn LX_v_required        : dword
extrn LX_v_actual          : dword
extrn LX_FixSelDPLPtr      : dword
extrn LX_RestoreSelDPLPtr  : dword
extrn LX_StackAllocPtr     : dword
extrn LX_StackFreePtr      : dword
extrn LX_FlatToSelPtr      : dword
extrn LX_IrqStackAllocPtr  : dword
extrn LX_IrqStackFreePtr   : dword
extrn tempeax              : dword
extrn tempedx              : dword
extrn tempesi              : dword
extrn cpuflags             : dword
extrn fInitStack           : dword
extrn intSwitchStack       : dword
DATA32 ENDS


DATA16 SEGMENT
IFNDEF NODEVHELPINITVAR
extrn DevHelpInit          : dword
ENDIF
DATA16 ENDS

