/* $Id: lxinit.c,v 1.67 2006/04/23 21:01:44 smilcke Exp $ */

/*
 * lxsinit.c
 * Autor:               Stefan Milcke
 * Erstellt am:         07.09.2001
 * Letzte Aenderung am: 23.04.2006
 *
*/

#define INCL_DOSPROCESS
#include <lxcommon.h>
#include <devrp.h>

#include "Ver_32.h"

#include <lxapilib.h>
#include <lxapi.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/init.h>

#include <lxdaemon.h>

extern int lx_do_verbose;
extern int lx_verbose_modulelist;

extern unsigned long build_level;

static char ERR_ERROR[] = "ERROR: ";
static char ERR_LOCK[] = "Unable to lock segment ";
static char ERR_CS[] = "CODE32.";
static char ERR_DS[] = "DATA32.";
static char ERR_CO[] = "CONST32_RO.";
static char ERR_EXIT[] = " Exiting...\n";
static char ERR_INITCS[] ="INITCODE32.";

extern WORD32 INIT_COUNT_PTR;

extern char lx_drv_homepath[];
extern char* drvparams;
extern int dolxdevtree;

//----------------------------- LX_set_logFileName -----------------------------
static inline void LX_set_logFileName(char* fileName)
{
 if(LX_DOSSCANENV("LOGFILES",lx_log_file_name)
    && LX_DOSSCANENV("TMP",lx_log_file_name))
  strcpy(lx_log_file_name,lx_drv_homepath);
 else
  strcat(lx_log_file_name,"\\");
 strcpy(lx_log_path,lx_log_file_name);
 strcat(lx_log_file_name,fileName);
 LX_DMN_SETLOGNAME(lx_log_file_name);
}

//------------------------------- LX_WriteString -------------------------------
VOID LX_WriteString(const char* str)
{
 char* msg;
 char* c;
 msg=(char*)kmalloc(strlen(str)*2,GFP_KERNEL);
 if(!msg)
  return;
 c=msg;
 while(*str)
 {
  if(*str=='\n')
   *c++='\r';
  *c++=*str++;
 }
 *c++=*str++;
 LX_scr_put_string(msg);
 kfree(msg);
 return;
}

//--------------------------------- LX_Verbose ---------------------------------
VOID LX_Verbose(const char* str)
{
 if(lx_do_verbose)
 {
  LX_WriteString((char*)str);
  if(P_INIT_COUNT && (*P_INIT_COUNT==255))
  {
   DevRun(INIT_COUNT_PTR);
//   DevBlock(INIT_COUNT_PTR,1,1);
  }
 }
}

//------------------------------ LX_writeLockErr -------------------------------
void LX_writeLockErr(char* msg,int rc)
{
 char buffer[100];
 LX_WriteString(ERR_ERROR);
 sprintf(buffer,"%d ",rc);
 LX_WriteString(buffer);
 LX_WriteString(ERR_LOCK);
 LX_WriteString(msg);
 LX_WriteString(ERR_EXIT);
}

extern unsigned long lx_kernel_ds_size;
extern unsigned long lx_kernel_cs_size;

extern BYTE _SEGSTRT_CONST32_RO;
extern BYTE _SEGEND_CONST32_RO;
#define OFF_STRT_CONST32_RO   (SEG_MAKE_OFF(_SEGSTRT_CONST32_RO))
#define OFF_END_CONST32_RO    (SEG_MAKE_OFF(_SEGEND_CONST32_RO))

extern unsigned long LX_lock_gcc_segments(void);

//------------------------------ LX_LockSegments -------------------------------
static inline int LX_LockSegments(void)
{
 int rc;
 unsigned long sz;
 lx_kernel_ds_size=(OFF_END_DATA32-OFF_STRT_DATA32);
 lx_kernel_cs_size=(OFF_END_TEXT32-OFF_STRT_TEXT32);
 /* Locks DATA32 into physical memory */
 rc=LX_lock_mem((ULONG)OFF_STRT_DATA32,lx_kernel_ds_size,VMDHL_LONG | VMDHL_WRITE);
 if(rc)
 {
  LX_writeLockErr(&(ERR_DS[0]),rc);
  return rc;
 }
 /* Locks CODE32 into physical memory */
 rc=LX_lock_mem((ULONG)OFF_STRT_TEXT32,lx_kernel_cs_size,VMDHL_LONG);
 if(rc)
 {
  LX_writeLockErr(&(ERR_CS[0]),rc);
  return rc;
 }
 /* Locks CONST32_RO into physical memory */
 sz=(OFF_END_CONST32_RO-OFF_STRT_CONST32_RO);
 rc=LX_lock_mem((ULONG)OFF_STRT_CONST32_RO,sz,VMDHL_LONG);
 if(rc)
  LX_writeLockErr(&(ERR_CO[0]),rc);
 else
  LX_lock_gcc_segments();
 return rc;
}

//------------------------------ LX_Init_PDDStack ------------------------------
static int LX_Init_PDDStack(struct PDDStack** pRoot
                            ,int num_stacks,int stack_size)
{
 struct PDDStack* p;
 struct PDDStack* pold;
 unsigned long base=(unsigned long)kmalloc(stack_size*num_stacks,GFP_KERNEL);
 p=(struct PDDStack*)kmalloc(sizeof(struct PDDStack)*num_stacks,GFP_KERNEL);
 if(0!=p && 0!=base)
 {
  int i;
  for(i=0;i<num_stacks;i++)
  {
   int tmp;
   p[i].next=&(p[i+1]);
   p[i].flags=0;
   p[i].base=base;
   p[i].pid=0;
   p[i].tid=0;
   base+=stack_size;
   memset((void*)p[i].base,0,sizeof(struct thread_info));
  }
  p[i-1].next=0;
  pold=*pRoot;
  *pRoot=p;
  if(pold)
  {
   kfree((void*)(pold[0].base));
   kfree(pold);
  }
 }
 else
 {
  if(base)
   kfree((void*)base);
  if(p)
   kfree((void*)p);
 }
 return 0;
}

//------------------------------ LX_ReInit_Stacks ------------------------------
int LX_ReInit_Stacks(unsigned long numIrqStacks)
{
 return LX_Init_PDDStack(&lx_IrqStackRoot,numIrqStacks,CONFIG_THREAD_SIZE);
}

extern void LX_parseArgs(struct LX_RP* prp);
extern unsigned long LX_initResourceManager(void);
extern void LX_init_init_task(void);

//-------------------------------- LX_InitVars ---------------------------------
static ULONG LX_InitVars(void)
{
 ULONG rc;
 ULONG ptr,lin;
 rc=0;
 if(!rc)
  if(0==(rc=DevGetDOSVar(1,0,&ptr)))
   if(0==(rc=DevVirtToLin(SELECTOROF(ptr),OFFSETOF(ptr),(LINEAR*)&lin)))
    if(0==(rc=DevVirtToLin((SEL)(*(PSHORT)lin),0,(LINEAR*)&lin)))
     lx_InfoSegGDTPtr=(struct InfoSegGDT*)lin;
 if(!rc)
  if(0==(rc=DevGetDOSVar(2,0,&ptr)))
   if(0==(rc=DevVirtToLin(SELECTOROF(ptr),OFFSETOF(ptr),(LINEAR*)&lin)))
   {
    if(0==(rc=DevVirtToLin(SELECTOROF(*(PULONG)lin),OFFSETOF(*(PULONG)lin),(LINEAR*)&lin)))
     lx_InfoSegLDTPtr=(struct InfoSegLDT*)lin;
   }
/*
 if(!rc)
  if(0==(rc=DevGetDOSVar(13,0,&ptr)))
   if(0==(rc=DevVirtToLin(SELECTOROF(ptr),OFFSETOF(ptr),(LINEAR*)&lin)))
    lx_IrqLevelPtr=(unsigned long*)lin;
*/
 if(rc)
 {
  LX_WriteString(ERR_ERROR);
  LX_WriteString("Unable to get DOSVar, exiting...\n");
 }
 else
 {
  // SMP check
  lx_is_smp=0;
  if(0==DevGetDOSVar(18,0,&ptr))
  {
   lin=LX_GET1616PTRCONTENTSL(ptr);
   if(lin)
   {
    lx_is_smp=1;
    if(0==DevGetDOSVar(17,0,&ptr))
    {
     lin=LX_GET1616PTRCONTENTSL(ptr);
     if(lin>64)
      lx_num_cpus=1;
     else
      lx_num_cpus=lin;
    }
   }
  }
  if(!rc)
   rc=LX_ReInit_Stacks(4);
  if(!rc)
   LX_init_init_task();
 }
 return rc;
}

extern WORD32 _OffsetFinalDS16;
extern WORD32 _OffsetFinalCS16;
extern void fastcall LXA_start_service_thread(unsigned long* tid);
extern void LX_start_init_thread(void);
extern unsigned long lxa_service_thread_id;

//-------------------------------- LX_StratInit --------------------------------
WORD32 LX_StratInit(struct LX_RP* prp)
{
 WORD32 rc=0;
 char buffer[100];
 prp->RPBODY.RPINITOUT.FinalCS = 0;
 prp->RPBODY.RPINITOUT.FinalDS = 0;
 if(!P_INIT_COUNT)
 {
  unsigned long p;
  DevVirtToLin(SELECTOROF(INIT_COUNT_PTR),OFFSETOF(INIT_COUNT_PTR)
               ,(LINEAR*)&p);
  P_INIT_COUNT=(unsigned short*)p;
 }
 if(P_INIT_COUNT)
 {
  switch(*P_INIT_COUNT)
  {
   case 1:
    LX_parseArgs(prp);
    LX_open_screen();
    {
     sprintf(buffer
             ,"LXAPI support driver V%d.%d Build %d linux(2.6.5)\n(C) 2001-2005 by Stefan Milcke\n"
             ,LX32_DRV_MAJOR_VERSION
             ,LX32_DRV_MINOR_VERSION
             ,BUILD_LEVEL);
     LX_Verbose(buffer);
    }
    LX_Verbose("Locking segments\n");
    break;
   case 2:
    if(LX_LockSegments())
    {
     LX_set_continue_startup();
     rc=RPERR;
    }
    else
     LX_Verbose("Initializing global variables\n");
    break;
   case 3:
    if(LX_InitVars())
    {
     LX_set_continue_startup();
     rc=RPERR;
    }
    else
    {
     LXA_start_service_thread(&lxa_service_thread_id);
     LX_Verbose("Waiting for daemon\n");
    }
    break;
   case 4:
    if(LX_wait_for_daemon())
    {
     LX_Verbose("Unable to start LXAPID daemon, exiting...\n");
     LX_set_continue_startup();
    }
    else
    {
     char stars[]="*******************************************************************************\n";
     LX_set_logFileName("LXAPI32.LOG");
     LX_write_log(stars);
#ifdef LXDEBUG
     sprintf(buffer,"Command line parameters:%s\n",drvparams);
     LX_write_log(buffer);
#endif
     sprintf(buffer,"LXAPI32 Logging started:%04d.%02d.%02d %02d:%02d:%02d\n"
            ,lx_InfoSegGDTPtr->SIS_YrsDate
            ,lx_InfoSegGDTPtr->SIS_MonDate
            ,lx_InfoSegGDTPtr->SIS_DayDate
            ,lx_InfoSegGDTPtr->SIS_HrsTime
            ,lx_InfoSegGDTPtr->SIS_MinTime
            ,lx_InfoSegGDTPtr->SIS_SecTime);
     LX_write_log(buffer);
     LX_write_log(stars);
     LX_Verbose("Initializing resource manager\n");
    }
    break;
   case 5:
    LX_initResourceManager();
    LX_Verbose("Starting init thread\n");
    break;
   case 6:
    LX_start_init_thread();
    break;
   case 7:
    *P_INIT_COUNT=255;
    break;
   default:
    break;
  }
 }
 // Complete the installation
 prp->RPBODY.RPINITOUT.FinalCS = _OffsetFinalCS16;
 prp->RPBODY.RPINITOUT.FinalDS = _OffsetFinalDS16;
 dolxdevtree=1;
 // Confirm a successful installation
 return RPDONE;
}
