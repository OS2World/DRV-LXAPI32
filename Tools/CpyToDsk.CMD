/* $Id: CpyToDsk.CMD,v 1.12 2005/04/04 22:39:57 smilcke Exp $ */

/* Build.CMD for LXAPI32 driver */
'@echo off'
setlocal

/* Drv='A:' */
Drv='\\LPSTM\FDRIVE\NETDRV'

/* Directories to copy */
Parse Arg Directories

/* Create Variable for LXApiEnv.CMD */
LXApiDir=Value('LXAPI32DEV',,'OS2ENVIRONMENT')
LXApiEnv=LXApiDir'\LXApiEnv.CMD'
TmpDir=Value('TEMP',,'OS2ENVIRONMENT')
rc=Stream(LXApiEnv,'C','QUERY EXISTS')
If Length(rc)<1 Then
Do
 Say 'Unable to locate LXApiEnv.CMD'
 Say 'Maybe Environment variable LXAPI32DEV is wrong'
 endlocal
 exit
End

'Call 'LXApiEnv
CopyToDisk=Value('LX_COPY_TO_DISK',,'OS2ENVIRONMENT')

/* TAR/GZIP Options */
/* AddParms='-c -v' */
AddParms='/Add=update /NoExtended /Max'
ExclPat='*.lst *.lss *.lrf *.lib *.err *.map *.bak *.dep *.obj lxsyms.s *.o'
/* ExclPat=ExclPat' *.c* *.h* *.mak makefile *.def *.txt readme *.cmd *.inc *.s *.asm *.sym LICENSE defconfig a.out.h boot init? Daemon Dev32 Inc Include Lib Linux lxapilib Makes os2 Root Tools' */
ExclParms=''
Do i=1 To Words(ExclPat)
 ExclParms=ExclParms' /Excl='Word(ExclPat,i)
End

/* Set subdirectory on A: if copy is enabled */
TargetDirectory=Directory()
If Translate(Right(TargetDirectory,5))='TOOLS' Then
Do
 TargetDirectory=Left(TargetDirectory,Length(TargetDirectory)-6)
 '@cd..'
End
Do While Pos('\',TargetDirectory)>0
 TargetDirectory=Right(TargetDirectory,Length(TargetDirectory)-1)
End
TargetDirectory='\'TargetDirectory

TargetDirectory='\'

TargetFile=Drv''TargetDirectory'LXAPI32.zip'

If Length(Drv)>2 Then
Do
 rc=SysFileTree(Drv'\*',T,'D')
End
Else
Do
 rc=SysDriveInfo(Drv)
End
If (rc<>'') Then
Do
 '@md 'Drv''TargetDirectory' 2> nul'
 'call pkzip 'AddParms' /Dir 'ExclParms' 'TargetFile
End
endlocal
Exit
