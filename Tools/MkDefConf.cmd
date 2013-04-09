/* $Id: MkDefConf.cmd,v 1.4 2005/05/12 20:54:47 smilcke Exp $ */

/* Generates defconf
   Copyright (c) 2004-2004 by Stefan Milcke
                              K�ferstra�e 45
                              28779 Bremen

   Datum der Erstellung: 24.06.2004
   Letzte Aenderung am:  06.11.2004
*/
Call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs';
Call SysLoadFuncs;

Parse Arg IncludeDir IncDir

InFile=IncludeDir'\defconfig'
Out1File=IncludeDir'\defconfig.h'
Out2File=IncDir'\defcfg.inc'

Say 'Generating 'Out1File' and 'Out2File
'@if exist 'Out1File' @del 'Out1File
'@if exist 'Out2File' @del 'Out2File
rc=Stream(InFile,'C','OPEN READ')
rc=Stream(Out1File,'C','OPEN WRITE')
rc=Stream(Out2File,'C','OPEM WRITE')
rc=LineOut(Out1File,'/* $id:$ */')
rc=LineOut(Out1File,'/* Generated by MkDefConf.cmd */')
rc=LineOut(Out1File,'/* 'Date()' 'Time()' */')
rc=LineOut(Out1File,'#ifndef CONFIG_H_INCLUDED')
rc=LineOut(Out1File,'#define CONFIG_H_INCLUDED')
rc=LineOut(Out1File,'')
rc=LineOut(Out2File,'; $Id: MkDefConf.cmd,v 1.4 2005/05/12 20:54:47 smilcke Exp $')
rc=LineOut(Out2File,'; Generated by MkDefConf.cmd')
rc=LineOut(Out2File,'; 'Date()' 'Time())
Do Until Lines(InFile)<1
 L=LineIn(InFile)
 If Left(L,7)='CONFIG_' Then
 Do
  V=Left(L,Pos('=',L)-1)
  C=Right(L,Length(L)-Pos('=',L))
  If C='y' Then
  Do
   L1='#define 'V
   L2=V' EQU 1'
  End
  If C<>'y' Then
  Do
   L1='#define 'V' ('C')'
   L2=V' EQU ('C')'
  End
  rc=LineOut(Out1File,L1)
  rc=LineOut(Out2File,L2)
 End
 Else
 Do
  L='//'L
/*  rc=LineOut(OutFile,L) */
 End
End
rc=LineOut(Out1File,'')
rc=LineOut(Out1File,'#endif // CONFIG_H_INCLUDED')
rc=LineOut(Out1File,'')
rc=Stream(Out1File,'C','CLOSE')
rc=Stream(Out2File,'C','CLOSE')
rc=Stream(InFile,'C','CLOSE')