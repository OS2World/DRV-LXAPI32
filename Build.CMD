/* $Id: Build.CMD,v 1.29 2004/10/31 23:46:52 smilcke Exp $ */

/* Build.CMD for LXAPI32 driver */
'@echo off'
'mode 80,80'
setlocal

Call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs';
Call SysLoadFuncs;
Call CheckEnvironment

LXApiDir=Value('LXAPI32DEV',,'OS2ENVIRONMENT')
LXApiEnv=LXApiDir'\LXApiEnv.CMD'
'Call 'LXApiEnv
CopyToDisk=Value('LX_COPY_TO_DISK',,'OS2ENVIRONMENT')
ErrViewer=Value('LX_ERR_VIEWER',,'OS2ENVIRONMENT')
If Length(ErrViewer)<1 Then ErrViewer='@type'

All=0
Clean=0
Release=0

Call SysFileTree '*.err','efi','FSO'
Do i=1 To efi.0
 Call SysFileDelete efi.i
End

makeopts='-w LXKERNEL=1'

Parse Arg Arguments
Do i=1 To Words(Arguments)
 A=Translate(Word(Arguments,i))
 If A='ALL' Then Clean=1
 If A='CLEAN' Then Clean=1
 If A='LXDEBUG' Then Release=0
 If A='RELEASE' Then Release=1
End

If All=1 Then '@call Build.cmd clean'
If All=1 Then makeopts=makeopts' all'
If Clean=1 Then makeopts=makeopts' clean'
If Release=0 Then
Do
 makeopts=makeopts' LXDEBUG=1'
 '@SET LXDEBUG=1'
End

'@if exist Bin\LXAPID.EXE @call unlock Bin\LXAPID.EXE'
'@if exist Bin\LXAPI32.LOG @del Bin\LXAPI32.LOG > nul'
/* Check, if previous build was release or not */
If Clean=0 Then
Do
 EA='LastBuild'
 rc=SysGetEA(Directory(),'LastBuild','WasRelease')
 If WasRelease='Debug' Then WasRelease=0
 If WasRelease='Release' Then WasRelease=1
 If (WasRelease<>Release) Then DoClean=1
 If DoClean=1 Then
 Do
  '@call Build Clean'
  '@call 'LXApiEnv
  If Release=1 Then
  Do
   rc=SysPutEA(Directory(),'LastBuild','Release')
  End
  Else
  Do
   rc=SysPutEA(Directory(),'LastBuild','Debug')
  End
 End
End

/* Get time stamp of old file */
'@copy .\tools\buildlvl.cmd .\tools\buildlvl.old > nul'
Call SysFileTree 'BIN\\*.SYS','ofi','F'
If Release=1 Then
Do
 If WasRelease<>Release Then
 Do
  '@call 'LXApiDir'\Tools\BldMgr.cmd /File:.\Tools\buildlvl.cmd /Text:BUILD_LEVEL= /Action:INCR /Quiet'
 End
end
'@call .\Tools\buildlvl.cmd'
'@make 'makeopts
If Clean=1 Then rc=1
Else If rc<>0 Then
Do
 tm=rc
 '@copy .\tools\buildlvl.old .\tools\buildlvl.cmd > nul'
 Call SysFileTree '*.err','efi','FSO'
 Do i=1 To efi.0
  '@'ErrViewer' 'efi.i
 End
 rc=tm
End
Else
Do
 IsNew=0
 Call SysFileTree 'BIN\\*.SYS','nfi','F'
 Do i=1 To nfi.0
  If nfi.i<>ofi.i Then IsNew=1
 End
 If IsNew=0 Then
 Do
  tm=rc
  '@copy .\tools\buildlvl.old .\tools\buildlvl.cmd > nul'
  rc=tm
 End
End
tm=rc
'@if exist .\tools\buildlvl.old del .\tools\buildlvl.old'
rc=tm
If Clean=1 Then CopyToDisk=0
If CopyToDisk=1 Then
Do
 If rc=0 Then '@Call CpyToDsk.CMD'
End
'@endlocal'
Exit

CheckEnvironment: Procedure
 rc1=CheckEnvPath('DDK','BASE\H\RMCALLS.H')
 rc2=CheckEnvPath('LXAPI32DEV','Tools\BldMgr.cmd')
 rc3=CheckEnvPath('GCC','include\a_out_stab.h')
 rc4=CheckEnvPath('EMXUTILS','make.exe')
 If rc1<1 | rc2<1 | rc3<1 | rc4<1 Then Exit
Return 0

CheckEnvPath: Procedure
 E=Arg(1)
 F=Arg(2)
 P=Value(E,,'OS2ENVIRONMENT')
 CF=P'\'F
 rc=SysFileTree(CF,FF,'F')
 If FF.0<1 Then
 Do
  Say 'Environment variable 'E' not defined or wrong'
 End
Return FF.0
