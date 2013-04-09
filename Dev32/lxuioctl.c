/* $Id: lxuioctl.c,v 1.17 2006/01/05 23:48:18 smilcke Exp $ */

/*
 * lxuioctl.c
 * Autor:               Stefan Milcke
 * Erstellt am:         07.03.2005
 * Letzte Aenderung am: 29.12.2005
 *
*/

#include <lxcommon.h>
#include <devrp.h>

#include <lxapi.h>

#include <linux/string.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <lxapilib.h>
#include <lxuapioctl.h>
#include <linux/unistd.h>
#include <linux/mm.h>

//-------------------------------- LX_SYSCALL0 ---------------------------------
ULONG LX_SYSCALL0(int callnum)
{
 long rc;
 __asm__ __volatile__
 (
  LXSYSCALLSTR ";"
  :"=a"(rc)
  :"0"(callnum)
 );
 return rc;
}

//-------------------------------- LX_SYSCALL1 ---------------------------------
ULONG LX_SYSCALL1(int callnum,ULONG arg1)
{
 long rc;
 __asm__ __volatile__
 (
  LXSYSCALLSTR ";"
  :"=a"(rc)
  :"0"(callnum), "b"(arg1)
 );
 return rc;
}

//-------------------------------- LX_SYSCALL2 ---------------------------------
ULONG LX_SYSCALL2(int callnum,ULONG arg1,ULONG arg2)
{
 long rc;
 __asm__ __volatile__
 (
  LXSYSCALLSTR ";"
  :"=a"(rc)
  :"0"(callnum), "b"(arg1), "c"(arg2)
 );
 return rc;
}

//-------------------------------- LX_SYSCALL3 ---------------------------------
ULONG LX_SYSCALL3(int callnum,ULONG arg1,ULONG arg2,ULONG arg3)
{
 long rc;
 __asm__ __volatile__
 (
  LXSYSCALLSTR ";"
  :"=a"(rc)
  :"0"(callnum), "b"(arg1), "c"(arg2), "d"(arg3)
 );
 return rc;
}

//-------------------------------- LX_SYSCALL4 ---------------------------------
ULONG LX_SYSCALL4(int callnum,ULONG arg1,ULONG arg2,ULONG arg3,ULONG arg4)
{
 long rc;
 __asm__ __volatile__
 (
  LXSYSCALLSTR ";"
  :"=a"(rc)
  :"0"(callnum), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4)
 );
 return rc;
}

//-------------------------------- LX_SYSCALL5 ---------------------------------
ULONG LX_SYSCALL5(int callnum,ULONG arg1,ULONG arg2,ULONG arg3,ULONG arg4
                  ,ULONG arg5)
{
 long rc;
 __asm__ __volatile__
 (
  LXSYSCALLSTR ";"
  :"=a"(rc)
  :"0"(callnum), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5)
 );
 return rc;
}

//-------------------------------- LX_SYSCALL6 ---------------------------------
/*
ULONG LX_SYSCALL6(int callnum,ULONG arg1,ULONG arg2,ULONG arg3,ULONG arg4
                  ,ULONG arg5,ULONG arg6)
{
 long rc;
 __asm__ __volatile__
 (
  "push  %%ebp;"
  "movl  %%eax,%%ebp;"
  "movl  %%1,%%eax;"
  LXSYSCALLSTR ";"
  "pop   %%ebp;"
  :"=a"(rc)
  :"i"((int)callnum), "0"(arg6), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5)
 );
 return rc;
}
*/

//------------------------------- LX_utl_syscall -------------------------------
static ULONG LX_utl_syscall(PLXIOCPA_SYS_CALL pa)
{
 // In case syscall doesn't return here we set LX_SYSCALL_RETFLAG_EXITPROCESS
 // This the case for execve!
 pa->retflags|=LX_SYSCALL_RETFLAG_EXITPROCESS;
// DevInt3();
 switch((pa->argc))
 {
  case 0:
   pa->result=LX_SYSCALL0(pa->syscallnum);
   break;
  case 1:
   pa->result=LX_SYSCALL1(pa->syscallnum,pa->arg1);
   break;
  case 2:
   pa->result=LX_SYSCALL2(pa->syscallnum,pa->arg1,pa->arg2);
   break;
  case 3:
   pa->result=LX_SYSCALL3(pa->syscallnum,pa->arg1,pa->arg2,pa->arg3);
   break;
  case 4:
   pa->result=LX_SYSCALL4(pa->syscallnum,pa->arg1,pa->arg2,pa->arg3,pa->arg4);
   break;
  case 5:
   pa->result=LX_SYSCALL5(pa->syscallnum,pa->arg1,pa->arg2,pa->arg3,pa->arg4
                          ,pa->arg5);
   break;
/*
  case 6:
   pa->result=LX_SYSCALL6(pa->syscallnum,pa->arg1,pa->arg2,pa->arg3,pa->arg4
                          ,pa->arg5,pa->arg6);
   break;
*/
  default:
   return -ENOENT;
 }
 pa->retflags&=~LX_SYSCALL_RETFLAG_EXITPROCESS;
 return pa->result;
}

//--------------------------------- ioctl_sys ----------------------------------
static ULONG ioctl_sys(struct LX_RP* prp)
{
 ULONG rc=0;
 switch(prp->RPBODY.RPIOCTL.Function)
 {
  case LXIOCFN_SYS_CALL:
  {
   PARMPACKET(pa,PLXIOCPA_SYS_CALL,prp,VERIFY_READWRITE);
   if(pa)
   {
    pa->result=LX_utl_syscall(pa);
    pa->sysstate=lx_sysstate;
   }
   else
    rc=RPERR_PARAMETER;
  }
 }
 return rc | RPDONE;
}

extern asmlinkage long sys_sync(void);
extern int shrink_all_memory(int);

//--------------------------------- ioctl_utl ----------------------------------
static ULONG ioctl_utl(struct LX_RP* prp)
{
 ULONG rc=0;
 switch(prp->RPBODY.RPIOCTL.Function)
 {
  case LXIOCFN_UTL_QUERY_SYSTEM_STATE:
   {
    PARMPACKET(pa,PLXIOCPA_UTL_QUERY_SYSTEM_STATE,prp,VERIFY_READWRITE);
    if(pa)
    {
     pa->sysstate=lx_sysstate;
     pa->rc=0;
    }
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_UTL_INIT_STARTED:
   LX_set_sysstate(LXSYSSTATE_INIT_STARTED,0);
   break;
  case LXIOCFN_UTL_INIT_FINISHED:
   {
    PARMPACKET(pa,PLXIOCPA_UTL_INIT_FINISHED,prp,VERIFY_READWRITE);
    if(pa)
    {
     if(0!=*P_INIT_COUNT)
     {
      if(pa->exit_flags&LXFIN_SHOWDRVSIZES)
       LX_print_memstat(1);
      while(shrink_all_memory(10000));
      if(pa->exit_flags&LXFIN_SYNC)
       sys_sync();
      if(pa->exit_flags&LXFIN_WAITFORSTABLE)
       LX_wait_for_stable_memstat(LX_WFSM_PRINTALL);
      while(shrink_all_memory(10000));
      if(pa->exit_flags&LXFIN_SHOWMEMSTATS)
       LX_print_memstat(0);
      LX_set_continue_startup();
      LX_set_sysstate(LXSYSSTATE_INIT_FINISHED,0);
      set_current_state(TASK_INTERRUPTIBLE);
      sleep_on(&lx_shutdown_wq);
      set_current_state(TASK_RUNNING);
      while(lxa_shutdown_level<2)
       schedule();
      pa->rc=0;
     }
     else
      pa->rc=ERROR_ALREADY_ASSIGNED;
    }
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_UTL_OS2PATHTOLX:
   {
    PARMPACKET(pa,PLXIOCPA_UTL_OS2PATHTOLX,prp,VERIFY_READWRITE);
    if(pa)
     pa->rc=LX_os2path_to_lx(pa->os2path,pa->lxpath);
    else
     rc=RPERR_PARAMETER;
   }
   break;
  default:
   rc=RPERR_COMMAND;
   break;
 }
 return rc | RPDONE;
}

//------------------------------- LX_UStratIOCtl -------------------------------
ULONG LX_UStratIOCtl(struct LX_RP* prp)
{
 ULONG rc=RPDONE;
 HLOCK hLock=0;
 current_fileId=prp->RPBODY.RPIOCTL.FileID;
 LX_set_current_dev_fileId(prp->RPBODY.RPIOCTL.FileID);
 if(prp->RPBODY.RPIOCTL.DataLength)
  DevSegLock(SELECTOROF(prp->RPBODY.RPIOCTL.DataPacket),1,0,&hLock);
 switch(prp->RPBODY.RPIOCTL.Category)
 {
  case LXIOCCAT_SYS:
   rc=ioctl_sys(prp);
   break;
  case LXIOCCAT_UTL:
   rc=ioctl_utl(prp);
   break;
  default:
   rc|=RPERR_COMMAND;
   break;
 }
 LX_set_current_dev_fileId(0);
 if(hLock && prp->RPBODY.RPIOCTL.DataLength)
  DevSegUnlock(hLock);
 return rc;
}
