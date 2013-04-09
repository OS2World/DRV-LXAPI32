/* $Id: CnvDep.cmd,v 1.3 2004/07/23 21:42:37 smilcke Exp $ */

/* Convert dependency file (Convert all .o: to .obj:) */
Call RxFuncAdd 'SysLoadFuncs','RexxUtil','SysLoadFuncs'
Call RxFuncAdd 'SysFileDelete','RexxUtil','SysFileDelete'
Call RxFuncAdd 'SysTempFileName','RexxUtil','SysTempFileName'

Parse Arg DepFile
DepCount=0
TmpDir=Value('TMP',,'OS2ENVIRONMENT')'\'
GCCDir=Translate(Value('GCC',,'OS2ENVIRONMENT'),'/','\')
LXDir=Translate(Value('LXAPI32DEV',,'OS2ENVIRONMENT'),'/','\')
If Length(GCCDir)<1 Then GCCDir='<NO DEPENDENCY>'
TmpFile=SysTempFileName(TmpDir'DEPEND??.???')
rc=Stream(DepFile,'C','OPEN READ')
If rc='READY:' Then
Do
 rc=Stream(TmpFile,'C','OPEN WRITE')
 Do While Lines(DepFile)>0
  L=LineIn(DepFile)
  Do While Pos('.o:',L)>0
   P=Pos('.o:',L)
   LI=Left(L,P)
   RE=Right(L,Length(L)-P-1)
   L=LI'obj'RE
   DepCount=DepCount+1
  End
  L=ReplaceString(L,GCCDir,'$(GCC)')
  L=ReplaceString(L,LXDir,'$(LXAPI32DEV)')
  rc=LineOut(TmpFile,L)
 End
 rc=Stream(TmpFile,'C','CLOSE')
 rc=Stream(DepFile,'C','CLOSE')
 '@copy 'TmpFile' 'DepFile' >nul'
 '@del 'TmpFile' 2>nul'
End
Say DepCount' Dependencies created'
Exit

ReplaceString: Procedure
 L=Arg(1)
 SS=Arg(2)
 RS=Arg(3)
 Do While Pos(SS,L)>0
  P=Pos(SS,L)
  LI=Left(L,P-1)
  RE=Right(L,Length(L)-P-Length(SS)+1)
  L=LI''RS''RE
 End
return L