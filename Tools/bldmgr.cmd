/* $Id: bldmgr.cmd,v 1.5 2003/08/03 21:14:58 smilcke Exp $ */

/* Copyright (c) 2002 by Stefan Milcke
                         Kferstraáe 45
		   28779 Bremen

   This Rexx-Script is Freeware
*/
Call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs';
Call SysLoadFuncs;

/* Preinitialize some variables */
file='BUILDLVL.CMD'
text='BUILD_LEVEL'
va=1
verbose=1
action=''
YYYY=Left(Date('S'),4)
MM=Left(Right(Date('S'),4),2)
DD=Right(Date('S'),2)

ArgError=0
ShowUsage=0

IgnoreCase=0

/* Get and parse arguments */
Parse Arg Arguments
Do i=1 To Words(Arguments)
 A=Word(Arguments,i)
 B=Translate(A)
 If Left(B,6)='/FILE:' Then file=Right(A,Length(A)-6)
 else If Left(B,7)='/TEXTI:' Then
 Do
  text=Right(A,Length(A)-7)
  IgnoreCase=1
 End
 else If Left(B,6)='/TEXT:' Then text=Right(A,Length(A)-6)
 else If Left(B,8)='/ACTION:' Then action=Translate(Right(A,Length(A)-8))
 else If Left(B,4)='UIET' Then verbose=0
 else
 Do
  ShowUsage='Unknown argument:'A
  ArgError=1
 End
End

If Length(text)>0 Then
Do
 If Left(text,1)='"' Then text=Right(text,Length(text)-1)
 If Right(text,1)='"' Then text=Left(text,Length(text)-1)
End
If ArgError=0 Then
Do
 actionvalid=1
 If Left(action,4)='INCR' Then
 Do
  If Length(action)>4 Then va=Right(action,Length(action)-5)
  action='INCR'
 End
 Else If Left(action,4)='DECR' Then
 Do
  If Length(action)>4 Then va=Right(action,Length(action)-5)
  action='DECR'
 End
 Else If Left(action,5)='QUERY' Then
 Do
 End
 Else If Left(action,7)='SETDATE' Then
 Do
  va=DD"."MM"."YYYY
  If Length(action)>7 Then va=Right(action,Length(action)-8)
 End
 Else If Left(action,7)='SETYEAR' Then
 Do
  va=YYYY
  If Length(action)>7 Then va=Right(action,Length(action)-8)
  action='SET'
 End
 Else If Left(action,8)='SETMONTH' Then
 Do
  va=MM
  If Length(action)>8 Then va=Right(action,Length(action)-9)
  action='SET'
 End
 Else If Left(action,6)='SETDAY' Then
 Do
  va=DD
  If Length(action)>6 Then va=Right(action,Length(action)-7)
  action='SET'
 End
 Else If Left(action,3)='SET' Then
 Do
  If Length(action)>3 Then va=Right(action,Length(action)-4)
  action='SET'
 End
 Else
 Do
  actionvalid=0
 End
 If actionvalid=1 Then Call PerformAction
 Else
 Do
  ShowUsage='Unknown /ACTION: option:'action
 End
End

If ShowUsage<>0 Then
Do
 Say ''
 Say ShowUsage
 Say ''
 Call Usage
End
Exit

PerformAction: Procedure Expose file text action va verbose
 tmpfile=SysTempFileName('bldmgr.???')
 found=0
 changed=0
 If Length(Stream(tmpfile,"C","QUERY EXISTS"))> 0 Then '@del 'tmpfile
 rc=Stream(file,"C","OPEN READ")
 If rc='READY:' Then
 Do
  rc=Stream(tmpfile,"C","OPEN WRITE")
  Do Forever
   If Lines(file)=0 Then Leave
   line=LineIn(file)
   If found=0 Then
   Do
    If IgnoreCase=0 Then
    Do
     P1=Pos(Translate(text),Translate(line))
    End
    Else
    Do
     P1=Pos(Translate(text),Translate(line))
    End
    If P1>0 Then
    Do
     found=1
     OLine=line
     L=Left(line,P1+Length(text)-1)
     line=Right(line,Length(line)-Length(L))
     Do Forever
      If (Left(line,1)<>' ') Then Leave
      L=L' '
      line=Right(line,Length(line)-1)
     End
     V='0'
     If action='SETDATE' Then
     Do
      V=Left(line,10)
      line=Right(line,Length(line)-10)
     End
     Else
     Do
      Do Forever
       If Length(line)=0 Then Leave
       If DataType(Left(line,1),'N')=0 Then Leave
       V=V''Left(line,1)
       line=Right(line,Length(line)-1)
      End
      V=Value(V)
     End
     If action='QUERY' Then
     Do
      Say V
      line=OLine
     End
     Else
     Do
      If action='SETDATE' Then OV=V
      Else OV=V+0
      If action='INCR' Then V=V+va
      else if action='DECR' Then V=V-va
      else if action='SET' Then V=va
      else if action='SETDATE' Then V=va
      If OV<>V Then Changed=1
      line=L''V''line
      If verbose=1 Then
      Do
       Say 'Old value:'OV' replaced by:'V
      End
     End
    End
   End
   rc=LineOut(tmpfile,line)
  End
 End
 rc=Stream(file,"C","CLOSE")
 rc=Stream(tmpfile,"C","CLOSE")
 If found+changed=2 Then
 Do
  '@copy 'tmpfile' 'file' > nul'
 End
 '@del 'tmpfile
Return

Usage: Procedure
 Say 'Usage: BLDMGR.CMD [Arguments]'
 Say 'Arguments can be one of this:'
 Say '/QUIET'
 Say '        Do not display messages'
 Say '/FILE:<FileName>'
 Say '        Input file name'
 Say '/TEXT:<Text>'
 Say '      Text to search for'
 Say '/TEXTI:<Text>'
 Say '      Same as /TEXT, but ignore case'
 Say '/ACTION:<INCR[:n] | DECR[:n] | QUERY | SETYEAR | SETMONTH | SETDAY | SET[n]>'
 Say '        Action to perform on file'
 Say '        INCR[:n]       Increments value after <Text> by n (default=1)'
 Say '        DECR[:n]       Decrements value after <Text> by n (default=1)'
 Say '        QUERY          Displays value after <Text>'
 Say '        SETYEAR[:YYYY] Sets year YYYY after <Text> (default=current)'
 Say '        SETMONTH[:MM]  Sets month MM after <Text> (default=current)'
 Say '        SETDAY[:DD]    Sets day DD after <Text> (default=current)'
 Say '        SET[:n]        Sets value after <Text> to n (default=1)'
Return
