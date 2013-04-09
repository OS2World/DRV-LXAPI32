/* $Id: GSegments.cmd,v 1.3 2006/04/27 21:17:55 smilcke Exp $ */

/* Generates files to include in asm for extra GCC segments */
Call RxFuncAdd 'SysLoadFuncs','RexxUtil','SysLoadFuncs'
Call RxFuncAdd 'SysFileDelete','RexxUtil','SysFileDelete'
Call RxFuncAdd 'SysTempFileName','RexxUtil','SysTempFileName'

Parse Arg CfgFile IncDir
If Length(CfgFile)<1 Then
Do
 Say 'No CfgFile'
 Exit
End

sclass='CODE'

If Length(IncDir)<1 Then
Do
 Say 'No IncDir'
 Exit
End

SegDefFile=IncDir'\segdef.inc'
StrtFile=IncDir'\segbegin.inc'
EndFile=IncDir'\segend.inc'
SegFile=IncDir'\segments.inc'

If Length(SegDefFile)<1 Then
Do
 Say 'No SegDefFile'
 Exit
End
If Length(StrtFile)<1 Then
Do
 Say 'No StrtFile'
 Exit
End

If Length(EndFile)<1 Then
Do
 Say 'No EndFile'
 Exit
End

'@if exist 'SegDefFile' @del 'SegDefFile
'@if exist 'StrtFile' @del 'StrtFile
'@if exist 'EndFile' @del 'EndFile
'@if exist 'SegFile' @del 'SegFile

rc=Stream(CfgFile,'C','OPEN READ')
If rc<>'READY:' Then
Do
 Say 'Unable to open 'CfgFile
 Exit
End

rc=Stream(SegDefFile,'C','OPEN WRITE')
rc=Stream(StrtFile,'C','OPEN WRITE')
rc=Stream(EndFile,'C','OPEN WRITE')
rc=Stream(SegFile,'C','OPEN WRITE')

rc=LineOut(SegFile,'DATA32 SEGMENT')
rc=LineOut(SegFile,'ASSUME CS:FLAT, DS:FLAT, ES:FLAT')
rc=LineOut(SegFile,'PUBLIC __code_segments')
rc=LineOut(SegFile,'__code_segments:')

Do Until Lines(CfgFile)<1
 L=LineIn(CfgFile)
 If Left(L,1)<>';' & Length(L)>1 Then
 Do
  if(L='[CODE]') Then
  Do
   sclass='CODE'
  End
  Else
  If(L='[DATA]') Then
  Do
   sclass='DATA'
   rc=LineOut(SegFile,'dd 0')
   rc=LineOut(SegFile,'dd 0')
   rc=LineOut(SegFile,'PUBLIC __data_segments')
   rc=LineOut(SegFile,'__data_segments:')
  End
  Else
  If(L='[CONST]') Then
  Do
   sclass='CONST'
  End
  Else
  Do
   If sclass='DATA' Then
   Do
    rc=LineOut(SegDefFile,L" SEGMENT DWORD PUBLIC USE32 '"L"'")
   End
   Else
   Do
    rc=LineOut(SegDefFile,L" SEGMENT DWORD PUBLIC USE32 '"sclass"'")
   End
/*   rc=LineOut(SegDefFile," ASSUME CS:FLAT, DS:FLAT, ES:FLAT") */
   rc=LineOut(SegDefFile,L" ENDS")

   rc=LineOut(StrtFile,"SEGBEGINMARKER "L)
   rc=LineOut(EndFile,"SEGENDMARKER "L)

   rc=LineOut(SegFile,"EXTRN __SEGEND_"L" : BYTE")
   rc=LineOut(SegFile,"dd OFFSET FLAT:__SEGSTRT_"L)
   rc=LineOut(SegFile,"dd OFFSET FLAT:__SEGEND_"L)
  End
 End
End
rc=LineOut(SegFile,'dd 0')
rc=LineOut(SegFile,'dd 0')
rc=LineOut(SegFile,'DATA32 ENDS')
Exit
