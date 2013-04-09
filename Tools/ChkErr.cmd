/* $Id: ChkErr.cmd,v 1.2 2004/05/26 20:32:54 smilcke Exp $ */

/* Check, if a valid error file was created */
Call RxFuncAdd 'SysLoadFuncs','RexxUtil','SysLoadFuncs'
Call RxFuncAdd 'SysFileDelete','RexxUtil','SysFileDelete'
Call RxFuncAdd 'SysTempFileName','RexxUtil','SysTempFileName'

Parse Arg ErrFile TypeIt DeleteIt Rem
If Length(ErrFile)<1 Then
Do
 Say 'No file specified'
 Exit
End
If Length(TypeIt)<1 Then TypeIt=0
If Length(DeleteIt)<1 Then DeleteIt=1
FS=Stream(ErrFile,'C','QUERY SIZE')
If Length(FS)=0 | FS<4 Then
Do
 '@If Exist 'ErrFile' @del 'ErrFile
End
Else
Do
 If TypeIt=1 Then
 Do
  'type 'ErrFile
 End
End

