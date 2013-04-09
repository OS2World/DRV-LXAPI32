/* $Id: lxparam.c,v 1.27 2006/02/16 23:07:14 smilcke Exp $ */

/*
 * lxparam.c
 * Autor:               Stefan Milcke
 * Erstellt am:         10.12.2001
 * Letzte Aenderung am: 05.02.2006
 *
*/

#include <lxcommon.h>
#include <devrp.h>

#include <linux/string.h>

#include <linux/module.h>

#include <lxapilib.h>
#include <lxdaemon.h>

#define MAX_PARM_LENGTH 256

int lx_do_verbose=0;
int lx_verbose_modulelist=0;

static char PARM_LENGTH_ERROR[]="LXAPI32: Parameter to long: ";
static char PARM_UNKNOWN[]="LXAPI32: Unknown parameter: ";
static char PARM_MALLOC_ERROR[]="LXAPI32: Memory allocation error\r\n";
static char CRLF[]="\r\n";

extern char lx_drv_homepath[];
char *drvparams=0;
extern int lx_do_last_startup_check;

static char *parmList[]=
{
 "/3",
 "/VM",
 "/V",
 "/NOSTARTCHECK"
};

static int numParms=sizeof(parmList)/sizeof(char *);

//------------------------------- LX_load_module -------------------------------
int LX_load_module(char *moduleName)
{
 int rc=0;
 if(moduleName)
  rc=0;
 return rc;
}

extern void LX_WriteString(const char *str);
extern int testfn(int testval);

//---------------------------------- parseArg ----------------------------------
int LX_parseArg(char* arg)
{
 int i;
 for(i=0;i<numParms;i++)
 {
  if(!strncmp(arg,parmList[i],strlen(parmList[i])))
  {
   switch(i)
   {
    case 0:
     DevInt3();
     break;
    case 1:
     lx_verbose_modulelist=1;
    case 2:
     lx_do_verbose=1;
     break;
    case 3:
     lx_do_last_startup_check=0;
     break;
   }
   break;
  }
 }
 if(i>=numParms)
 {
  LX_WriteString(PARM_UNKNOWN);
  LX_WriteString(arg);
  LX_WriteString(CRLF);
  return 1;
 }
 return 0;
}

//--------------------------------- parseArgs ----------------------------------
void LX_parseArgs(struct LX_RP* prp)
{
 char *args;
 char *carg;
 char *p;
 char *sl=NULL;
 int i;
 carg=(char*)kmalloc(MAX_PARM_LENGTH,GFP_KERNEL);
 if(carg)
 {
  if(0==DevVirtToLin(SELECTOROF(prp->RPBODY.RPINITIN.Args)
                     ,OFFSETOF(prp->RPBODY.RPINITIN.Args)
                     ,(LINEAR*)&args))
  {
   // Remove leading blanks
   while(*args && *args==' ')
    args++;
   // skip device=... statement while storing the path into lx_drv_homepath
   p=lx_drv_homepath;
   while(*args && *args!=' ')
   {
    *p=*args++;
    if(*p=='\\')
     sl=p;
    p++;
   }
   if(sl)
    *++sl=(char)0;
   drvparams=args;
   while(*args)
   {
    p=carg;
    while(*args && *args==' ') args++;
    i=0;
    while(*args && *args!=' ' && i<MAX_PARM_LENGTH-1)
    {
     *p++=*args++;
     i++;
    }
    if(i>=MAX_PARM_LENGTH-1)
    {
     carg[MAX_PARM_LENGTH-1]=(char)0;
     LX_WriteString(PARM_LENGTH_ERROR);
     LX_WriteString(carg);
     LX_WriteString(CRLF);
    }
    else
    {
     *p=(char)0;
     if(strlen(carg))
      if(LX_parseArg(carg))
       break;
    }
   }
  }
  kfree(carg);
 }
 else
  LX_WriteString(PARM_MALLOC_ERROR);
}
