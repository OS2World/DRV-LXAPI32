/* $Id: clean.cmd,v 1.7 2005/05/12 20:54:38 smilcke Exp $ */

/* Clean lxapi32 driver directories */

'@setlocal'
Call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs';
Call SysLoadFuncs;

LXApiDir=Value('LXAPI32DEV',,'OS2ENVIRONMENT')
If Length(LXApiDir)<1 Then
Do
 Say 'Environment variable LXAPI32DEV not found'
 '@endlocal'
 exit
End
LXApiEnv=LXApiDir'\LXApiEnv.CMD'
'@Call 'LXApiEnv
'@call make clean LXCLEAN=1'
rc=SysFileTree('*.dep','fi','FSO')
Do i=1 To fi.0
 rc=SysFileDelete(fi.i)
End
rc=SysFileTree('*.err','fi','FSO')
Do i=1 To fi.0
 rc=SysFileDelete(fi.i)
End
'@endlocal'
