/* $Id: GCSyms.cmd,v 1.9 2006/04/27 21:17:55 smilcke Exp $ */

/* Convers symbols with the EXPORT_SYMBOL and MODULE_PARM macro to an ASM file */
Call RxFuncAdd 'SysLoadFuncs','RexxUtil','SysLoadFuncs'
Call RxFuncAdd 'SysFileDelete','RexxUtil','SysFileDelete'
Call RxFuncAdd 'SysTempFileName','RexxUtil','SysTempFileName'

IsModule=0
GenEmpty=0
GenKallsyms=0
MapFile=''
OutFile=''
MasFile=''

Parse Arg Arguments
Do i=1 To Words(Arguments)
 A=Word(Arguments,i)
 TA=Translate(A)
 If TA='/GENEMPTY' Then GenEmpty=1
 Else If TA='/GENKALLSYMS' Then GenKallsyms=1
 Else If TA='/MODULE' Then IsModule=1
 Else If Left(TA,9)='/MAPFILE:' Then MapFile=Right(A,Length(A)-9)
 Else If Left(TA,9)='/MAPFILE=' Then MapFile=Right(A,Length(A)-9)
 Else If Left(TA,9)='/OUTFILE:' Then OutFile=Right(A,Length(A)-9)
 Else If Left(TA,9)='/OUTFILE=' Then OutFile=Right(A,Length(A)-9)
 Else Say 'Unknown Option:'A
End

If GenEmpty=0 Then
Do
 If MapFile='' Then
 Do
  Say 'No Mapfile'
  Call Usage
  Exit
 End
End

If OutFile='' Then
Do
 Say 'No File Stem'
 Call Usage
 Exit
End
OutAFile=OutFile'.S'

NoSymbol='kallsyms_addresses kallsyms_names kallsyms_num_syms __start___ksymtab __stop___ksymtab __start___ksymtab_gpl __stop___ksymtab_gpl __setup_start __setup_end __initcall_start __initcall_end __start___param __stop___param'
KeyWords='LXRX_EXPORT_SYM LXRX_SETUP_ LXRX_MODPARM_ LXRX_INITCALL_ LXRX_EXITCALL_ LXRX_SZ_PERCPU_ LXRX_CONS_INITCALL_'

If GenEmpty=0 Then
Do
 rc=Stream(MapFile,'C','OPEN READ')
 If rc<>'READY:' Then
 Do
  Say 'Mapfile 'MapFile' not found!'
  Exit
 End
End

'@if exist 'OutAFile' @del 'OutAFile

If GenEmpty=0 Then
Do
 MasFile=Left(MapFile,Length(MapFile)-3)'mas'
 '@if exist 'MasFile' @del 'MasFile
 rc=Stream(MasFile,'C','OPEN WRITE')
 Do Until Lines(MapFile)<1
  L=LineIn(MapFile)
  If L=' Origin   Group' Then Leave
  If Pos(':',L)=6 Then
  Do
   rc=LineOut(MasFile,L)
  End
 End
 rc=Stream(MasFile,'C','CLOSE')
 rc=Stream(MapFile,'C','CLOSE')
 rc=Stream(MapFile,'C','OPEN READ')
End


Kallsyms=0
Syms=0
rc=Stream(OutAFile,'C','OPEN WRITE')
rc=WriteHeader()

If GenEmpty=1 Then
 rc=WriteEmpty()
Else
Do
 Do Until Lines(MapFile)<1
  L=LineIn(MapFile)
  If L='  Address         Publics by Value' Then
  Do
   L=LineIn(MapFile)
   L=LineIn(MapFile)
   Do Until Lines(MapFile)<1
    L=LineIn(MapFile)
    If Length(L)<1 Then Leave
    If Words(L)=2 Then
    Do
     S=Word(L,2)
     If Left(S,1)='_' Then
     Do
      DoIt=1
      S=Right(S,Length(S)-1)
      Do i=1 To Words(NoSymbol)
       If Word(NoSymbol,i)=S Then DoIt=0
      End
      If DoIt=1 Then rc=ParseSymbol(S)
     End
    End
   End
  End
 End
 Do i=0 To Syms-1
  Do j=i To Syms-1
   If Sym.i>Sym.j Then
   Do
    T=Sym.i
    Sym.i=Sym.j
    Sym.j=T

    T=SymU.i
    SymU.i=SymU.j
    SymU.j=T

    T=SymT.i
    SymT.i=SymT.j
    SymT.j=T
   End
  End
 End
 rc=WriteSymbols()
End


If GenEmpty=0 Then rc=Stream(MapFile,'C','CLOSE')
rc=Stream(OutAFile,'C','CLOSE')
Exit


ParseSymbol: Procedure Expose Kallsyms Syms Kallsym. KallsymT. Sym. SymT. SymU. KeyWords
 S=Arg(1)
 KallsymT.Kallsyms=0
 Do c=1 To Words(KeyWords)
  If Pos(Word(KeyWords,c),S)=1 Then
  Do
   Sym.Syms=S
   SymT.Syms=c
   SymU.Syms=0
   Syms=Syms+1
   KallsymT.Kallsyms=c
  End
  Else If Pos('_'Word(KeyWords,c),S)=1 Then
  Do
   Sym.Syms=S
   SymT.Syms=c
   SymU.Syms=1
   Syms=Syms+1
   KallsymT.Kallsyms=c
  End
 End
 Kallsym.Kallsyms=S
 Kallsyms=Kallsyms+1
Return rc

WriteSymbols: Procedure  Expose IsModule Kallsyms Syms Kallsym. KallsymT. Sym. SymT. SymU. KeyWords OutAFile GenKallsyms
 rc=LineOut(OutAFile,'/* Init call table */')
 rc=LineOut(OutAFile,"/* This table is sorted so we don't need check level */")
 rc=LineOut(OutAFile,'.data')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl ___initcall_start')
 rc=LineOut(OutAFile,'	___initcall_start:')
 Do s=0 To Syms-1
  If SymT.s=4 Then rc=LineOut(OutAFile,'.long _'Sym.s)
 End
 rc=LineOut(OutAFile,'.globl ___initcall_end')
 rc=LineOut(OutAFile,'	___initcall_end:')
 rc=LineOut(OutAFile,'')

 rc=LineOut(OutAFile,'/* Console init call table */')
 rc=LineOut(OutAFile,'.data')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl ___con_initcall_start')
 rc=LineOut(OutAFile,'	___con_initcall_start:')
 Do s=0 To Syms-1
  If SymT.s=7 Then rc=LineOut(OutAFile,'.long _'Sym.s)
 End
 rc=LineOut(OutAFile,'.globl ___con_initcall_end')
 rc=LineOut(OutAFile,'	___con_initcall_end:')
 rc=LineOut(OutAFile,'')


 rc=LineOut(OutAFile,'/* Setup table */')
 rc=LineOut(OutAFile,'#ifndef TARGET_OS2_ELF')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl ___setup_start')
 rc=LineOut(OutAFile,'	___setup_start:')
 Do s=0 To Syms-1
  If SymT.s=2 Then
  Do
   If SymU.s=1 Then W=Right(Sym.s,Length(Sym.s)-12)
   Else W=Right(Sym.s,Length(Sym.s)-11)
   r=LineOut(OutAFile,'.long ___setup_str_'W)
   rc=LineOut(OutAFile,'.long _'Sym.s)
  End
 End
 rc=LineOut(OutAFile,'.globl ___setup_end')
 rc=LineOut(OutAFile,'	___setup_end:')
 rc=LineOut(OutAFile,'#endif //TARGET_OS2_ELF')
 rc=LineOut(OutAFile,'')

 rc=LineOut(OutAFile,'/* Per CPU data pointers */')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl ___per_cpu_start')
 rc=LineOut(OutAFile,' ___per_cpu_start:')
 Do s=0 To Syms-1
  If SymT.s=6 Then
  Do
   rc=LineOut(OutAFile,'.long _'Sym.s)
   rc=LineOut(OutAFile,'.long _per_cpu__'Right(Sym.s,Length(Sym.s)-15))
   rc=LineOut(OutAFile,'.long _LXRX_PERCPU__'Right(Sym.s,Length(Sym.s)-15))
  End
 End
 rc=LineOut(OutAFile,'.globl ___per_cpu_end')
 rc=LineOut(OutAFile,' ___per_cpu_end:')
 rc=LineOut(OutAFile,'')

 rc=LineOut(OutAFile,'/* Kernel param table */')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl ___start___param')
 rc=LineOut(OutAFile,'	___start___param:')
 rc=LineOut(OutAFile,'.globl ___stop___param')
 rc=LineOut(OutAFile,'	___stop___param:')
 rc=LineOut(OutAFile,'')

 rc=LineOut(OutAFile,'/* Exported symbols */')
 rc=LineOut(OutAFile,'#ifndef TARGET_OS2_ELF')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl ___start___ksymtab')
 rc=LineOut(OutAFile,' ___start___ksymtab:')
 Do s=0 To Syms-1
  If SymT.s=1 Then
  Do
   rc=LineOut(OutAFile,'.long _'Right(Sym.s,Length(Sym.s)-16))
   rc=LineOut(OutAFile,'.long _'Sym.s)
  End
 End
 rc=LineOut(OutAFile,'.globl ___stop___ksymtab')
 rc=LineOut(OutAFile,' ___stop___ksymtab:')
 rc=LineOut(OutAFile,'')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl ___start___ksymtab_gpl')
 rc=LineOut(OutAFile,' ___start___ksymtab_gpl:')
 rc=LineOut(OutAFile,'.globl ___stop___ksymtab_gpl')
 rc=LineOut(OutAFile,' ___stop___ksymtab_gpl:')
 rc=LineOut(OutAFile,'#endif //TARGET_OS2_ELF')
 rc=LineOut(OutAFile,'')

 rc=LineOut(OutAFile,'/* Kernel kallsym table */')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl _kallsyms_addresses')
 rc=LineOut(OutAFile,'	_kallsyms_addresses:')
 If GenKallsyms<>0 Then
 Do s=0 To Kallsyms-1
  rc=LineOut(OutAFile,'.long _'Kallsym.s)
 End
 rc=LineOut(OutAFile,'.globl _kallsyms_num_syms')
 rc=LineOut(OutAFile,'	_kallsyms_num_syms:')
 If GenKallsyms<>0 Then rc=LineOut(OutAFile,'.long 'Kallsyms)
 Else rc=LineOut(OutAFile,'.long 0')
 rc=LineOut(OutAFile,'.globl _kallsyms_names')
 rc=LineOut(OutAFile,'	_kallsyms_names:')
 If GenKallsyms<>0 Then
 Do
  Do s=0 To Kallsyms-1
   rc=LineOut(OutAFile,'.asciz "\0'Kallsym.s'"')
  End
  rc=LineOut(OutAFile,'.ascii "\0\0\0"')
 End
 rc=LineOut(OutAFile,'')

Return rc

WriteEmpty: Procedure Expose IsModule OutAFile
 /* Collect exported symbols so we don't miss a OPTFUNC */
 rc=SysFileTree('..\*.lss',lssf,'FSO')
 Syms.0=0
 Do i=1 To lssf.0
  F=lssf.i
  rc=Stream(F,'C','OPEN READ')
  Do Until Lines(F)<1
   L=LineIn(F)
   S=Word(L,2)
   If Left(S,16)='_LXRX_EXPORT_SYM' Then
   Do
    C=Syms.0+1
    Syms.C=S
    C=C+1
    S=Right(S,Length(S)-16)
    Syms.C=S
    Syms.0=C
   End
  End
  rc=Stream(F,'C','CLOSE')
 End
 rc=LineOut(OutAFile,'/* Init call table */')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl ___initcall_start')
 rc=LineOut(OutAFile,'	___initcall_start:')
 rc=LineOut(OutAFile,'.globl ___initcall_end')
 rc=LineOut(OutAFile,'	___initcall_end:')
 rc=LineOut(OutAFile,'')

 rc=LineOut(OutAFile,'/* Console init call table */')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl ___con_initcall_start')
 rc=LineOut(OutAFile,' ___con_initcall_start:')
 rc=LineOut(OutAFile,'.globl ___con_initcall_end')
 rc=LineOut(OutAFile,' ___con_initcall_end:')
 rc=LineOut(OutAFile,'')

 rc=LineOut(OutAFile,'/* Setup table */')
 rc=LineOut(OutAFile,'#ifndef TARGET_OS2_ELF')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl ___setup_start')
 rc=LineOut(OutAFile,'	___setup_start:')
 rc=LineOut(OutAFile,'.globl ___setup_end')
 rc=LineOut(OutAFile,'	___setup_end:')
 rc=LineOut(OutAFile,'#endif // TARGET_OS2_ELF')
 rc=LineOut(OutAFile,'')

 rc=LineOut(OutAFile,'/* Per CPU data pointers */')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl ___per_cpu_start')
 rc=LineOut(OutAFile,' ___per_cpu_start:')
 rc=LineOut(OutAFile,'.globl ___per_cpu_end')
 rc=LineOut(OutAFile,' ___per_cpu_end:')
 rc=LineOut(OutAFile,'')

 rc=LineOut(OutAFile,'/* Kernel param table */')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl ___start___param')
 rc=LineOut(OutAFile,'	___start___param:')
 rc=LineOut(OutAFile,'.globl ___stop___param')
 rc=LineOut(OutAFile,'	___stop___param:')
 rc=LineOut(OutAFile,'')

 rc=LineOut(OutAFile,'/* Exported symbols */')
 rc=LineOut(OutAFile,'#ifndef TARGET_OS2_ELF')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl ___start___ksymtab')
 rc=LineOut(OutAFile,' ___start___ksymtab:')
 Do i=1 To Syms.0
  rc=LineOut(OutAFile,'.long 'Syms.i)
 End
 rc=LineOut(OutAFile,'.globl ___stop___ksymtab')
 rc=LineOut(OutAFile,' ___stop___ksymtab:')
 rc=LineOut(OutAFile,'')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl ___start___ksymtab_gpl')
 rc=LineOut(OutAFile,' ___start___ksymtab_gpl:')
 rc=LineOut(OutAFile,'.globl ___stop___ksymtab_gpl')
 rc=LineOut(OutAFile,' ___stop___ksymtab_gpl:')
 rc=LineOut(OutAFile,'#endif // TARGET_OS2_ELF')
 rc=LineOut(OutAFile,'')

 rc=LineOut(OutAFile,'/* Kernel kallsym table */')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl _kallsyms_addresses')
 rc=LineOut(OutAFile,'	_kallsyms_addresses:')
 rc=LineOut(OutAFile,'.globl _kallsyms_num_syms')
 rc=LineOut(OutAFile,'	_kallsyms_num_syms:')
 rc=LineOut(OutAFile,'.globl _kallsyms_names')
 rc=LineOut(OutAFile,'	_kallsyms_names:')
 rc=LineOut(OutAFile,'.long 0')
 rc=LineOut(OutAFile,'')

Return rc

WriteHeader: Procedure Expose OutAFile
 DA=Date(S)
 rc=LineOut(OutAFile,'/* Generated by GCSyms.CMD */')
 rc=LineOut(OutAFile,'#include <defconfig.h>')
 rc=LineOut(OutAFile,'')
 rc=LineOut(OutAFile,'/* Build date */')
 rc=LineOut(OutAFile,'.data')
 rc=LineOut(OutAFile,'.align 4')
 rc=LineOut(OutAFile,'.globl _lx_BuildDay')
 rc=LineOut(OutAFile,' _lx_BuildDay:')
 rc=LineOut(OutAFile,'.long 'Right(DA,2)+0)
 rc=LineOut(OutAFile,'.globl _lx_BuildMonth')
 rc=LineOut(OutAFile,' _lx_BuildMonth:')
 rc=LineOut(OutAFile,'.long 'Left(Right(DA,4),2)+0)
 rc=LineOut(OutAFile,'.globl _lx_BuildYear')
 rc=LineOut(OutAFile,' _lx_BuildYear:')
 rc=LineOut(OutAFile,'.long 'Left(DA,4)+0)
 rc=LineOut(OutAFile,'')
Return rc

Usage:
 Say 'GCSyms </GENEMPTY> </MODULE> /MAPFILE:(FILE) /OUTFILE:(FILE)'
Return 0
