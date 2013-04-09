@REM $Id: LXApiEnv.cmd,v 1.13 2005/12/06 22:26:32 smilcke Exp $

@ECHO OFF

@IF %LXAPIENV_CALLED%!==! Goto DoIt
Goto EndIt

:DoIt
SET LXAPIENV_CALLED=1
@REM ***************************************************************************
@REM * For successfull build some environment variables must be set.
@REM * We assume that this is done in CONFIG.SYS
@REM * You can override this variables here (Except LXAPI32DEV)
@REM ***************************************************************************
@REM SET DDK=
@REM SET OBJ_BASE=

@REM Save old directories
SET OLD_INCLUDE=%INCLUDE%
SET OLD_LIB=%LIB%
SET OLD_PATH=%PATH%

@REM Set LX_COPY_TO_DISK to 1 if you want to copy files to disk after build
@REM This is usefull for remote debugging
SET LX_COPY_TO_DISK=1

@REM Set LX_ERR_VIEWER to your preferred viewer. It will popup if an error
@REM occours during compilation
@REM SET LX_ERR_VIEWER=E.EXE
SET LX_ERR_VIEWER=CMD.EXE /C E:\AGE40\BIN\SETENV.CMD CPPXLX40.EXE

@REM Setup DDK Path
SET PATH=%DDK%\tools;%DDK%\IBMC\CBIN;%PATH%

@REM Setup GCC Environment
CALL %GCC%\BIN\GCCENV.CMD %GCC% LINK386
SET PATH=%PATH%;%EMXUTILS%
SET BEGINLIBPATH=%BEGINLIBPATH%;%EMXUTILS%
SET LIBRARY_PATH=%LIBRARY_PATH%;%LXAPI32DEV%\lxlibc\lib

@REM Setup LXAPI Path
SET PATH=.\Tools;%LXAPI32DEV%\Tools;%PATH%

@SET PATH=%DDK%\base\tools;%PATH%
@SET LIB=%DDK%\base\lib;%LXAPI32DEV%\lxlibc\lib;%LIBRARY_PATH%;%LIB%
@SET INCLUDE=%DDK%\base\h;%INCLUDE%

@SET LIB=.\bin;%LIB%

@REM Only for GCC-ELF
SET PATH=E:\GCCELF;%PATH%

:EndIt
