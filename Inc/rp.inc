; $Id: rp.inc,v 1.5 2003/04/05 00:15:48 smilcke Exp $

;
; rp.inc
; Autor:               Stefan Milcke
; Erstellt am:         06.01.2002
; Letzte Aenderung am: 06.01.2002
;

reqPacket      struc
reqLenght      db ?
reqUnit        db ?
reqCommand     db ?
reqStatus      dw ?
reqFlags       db ?
               db 3 dup (?)    ; Reserved field
reqLink        dd ?
reqPacket      ends

rpInitIn       struc
i_rph          db size reqPacket dup (?)
i_unit         db ?
i_devHelp      dd ?
i_initArgs     dd ?
i_driveNum     db ?
rpInitIn       ends

rpInitOut      struc
o_rph          db size reqPacket dup (?)
o_unit         db ?
o_codeend      dw ?
o_dataend      dw ?
o_bpbarray     dd ?
o_status       dw ?
rpInitOut      ends

rpIOCtl        struc
ioctl_rph      db size reqPacket dup (?)
ioctl_Category db ?
ioctl_Function db ?
ioctl_Param    dw ?
ioctl_Data     dw ?
rpIOCtl        ends

