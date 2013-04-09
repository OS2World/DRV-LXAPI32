/* $Id: ExtrSyms.cmd,v 1.3 2006/02/16 23:07:19 smilcke Exp $ */

/* Extracts DEFINED SYMBOLS from LST file */
Call RxFuncAdd 'SysLoadFuncs','RexxUtil','SysLoadFuncs'
Call RxFuncAdd 'SysFileDelete','RexxUtil','SysFileDelete'
Call RxFuncAdd 'SysTempFileName','RexxUtil','SysTempFileName'

Parse Arg FileBase

StoreSeg='SYMPARM'

If Length(FileBase)<1 Then
Do
 Say 'No FileBase'
 Exit
End
IFile=FileBase'.lst'
OFile=FileBase'.lss'
'@if exist 'OFile' @del 'OFile

rc=Stream(IFile,'C','OPEN READ')
rc=Stream(OFile,'C','OPEN WRITE')
Do Until Lines(IFile)<1
 L=LineIn(IFile)
 If L='DEFINED SYMBOLS' Then Leave
End
If L='DEFINED SYMBOLS' Then
Do
 Do Until Lines(IFile)<1
  L=LineIn(IFile)
  If Length(L)>0 Then
  Do
   If Words(L)>2 Then
   Do
    W1=Word(L,2)
    W2=Word(L,3)
    If Pos(':',W2)=0 Then
    Do
     If Pos(':',W1)<>0 Then
     Do
      seg=Left(W1,Pos(':',W1)-1)
      If Left(W1,1)='.' Then rc=LineOut(OFile,seg' 'Word(L,3))
/*
      If Left(W1,5)='.text'      Then rc=LineOut(OFile,'.text 'Word(L,3))
      Else If Left(W1,5)='.data' Then rc=LineOut(OFile,'.data 'Word(L,3))
*/
     End
    End
   End
  End
 End
End

rc=Stream(IFile,'C','CLOSE')
rc=Stream(OFile,'C','CLOSE')
/*
'del 'IFile
'ren 'OFile' *.lst'
*/