/* $Id: lxlcsyscalls.c,v 1.5 2005/04/04 22:40:00 smilcke Exp $ */

/*
 * lxlcsyscalls.c
 * Autor:               Stefan Milcke
 * Erstellt am:         19.03.2005
 * Letzte Aenderung am: 02.04.2005
 *
*/

#define INCL_DOSDEVICES

#include <lxlibc.h>

#define __KERNEL__
#include <linux/config.h>
#include <linux/errno.h>
#include <linux/unistd.h>

#undef __syscall_return
#undef _syscall0
#undef _syscall1
#undef _syscall2
#undef _syscall3
#undef _syscall4
#undef _syscall5
#undef _syscall6

#define SYSCALL_PACKET0(name,num)\
 struct _LXIOCPA_SYS_CALL name={    \
  .result=-ENOENT,                  \
  .syscallnum=(unsigned long)(num), \
  .argc=0,                          \
}

#define SYSCALL_PACKET1(name,num,a1)\
 struct _LXIOCPA_SYS_CALL name={    \
  .result=-ENOENT,                  \
  .syscallnum=(unsigned long)(num), \
  .argc=1,                          \
  .arg1=(unsigned long)(a1),        \
}

#define SYSCALL_PACKET2(name,num,a1,a2)\
 struct _LXIOCPA_SYS_CALL name={    \
  .result=-ENOENT,                  \
  .syscallnum=(unsigned long)(num), \
  .argc=2,                          \
  .arg1=(unsigned long)(a1),        \
  .arg2=(unsigned long)(a2),        \
}

#define SYSCALL_PACKET3(name,num,a1,a2,a3)\
 struct _LXIOCPA_SYS_CALL name={    \
  .result=-ENOENT,                  \
  .syscallnum=(unsigned long)(num), \
  .argc=3,                          \
  .arg1=(unsigned long)(a1),        \
  .arg2=(unsigned long)(a2),        \
  .arg3=(unsigned long)(a3),        \
}

#define SYSCALL_PACKET4(name,num,a1,a2,a3,a4)\
 struct _LXIOCPA_SYS_CALL name={    \
  .result=-ENOENT,                  \
  .syscallnum=(unsigned long)(num), \
  .argc=4,                          \
  .arg1=(unsigned long)(a1),        \
  .arg2=(unsigned long)(a2),        \
  .arg3=(unsigned long)(a3),        \
  .arg4=(unsigned long)(a4),        \
}

#define SYSCALL_PACKET5(name,num,a1,a2,a3,a4,a5)\
 struct _LXIOCPA_SYS_CALL name={    \
  .result=-ENOENT,                  \
  .syscallnum=(unsigned long)(num), \
  .argc=5,                          \
  .arg1=(unsigned long)(a1),        \
  .arg2=(unsigned long)(a2),        \
  .arg3=(unsigned long)(a3),        \
  .arg4=(unsigned long)(a4),        \
  .arg5=(unsigned long)(a5),        \
}

#define SYSCALL_PACKET6(name,num,a1,a2,a3,a4,a5,a6)\
 struct _LXIOCPA_SYS_CALL name={    \
  .result=-ENOENT,                  \
  .syscallnum=(unsigned long)(num), \
  .argc=6,                          \
  .arg1=(unsigned long)(a1),        \
  .arg2=(unsigned long)(a2),        \
  .arg3=(unsigned long)(a3),        \
  .arg4=(unsigned long)(a4),        \
  .arg5=(unsigned long)(a5),        \
  .arg6=(unsigned long)(a6),        \
}

//---------------------------------- syscall0 ----------------------------------
LXAPISYSCRET LXAPIENTRY syscall0(int num)
{
 if(lxapi_rt_hfile)
 {
  ULONG plen,dlen,rc;
  SYSCALL_PACKET0(sysc,num);
  plen=sizeof(LXIOCPA_SYS_CALL);
  dlen=0;
  rc=DosDevIOCtl(lxapi_rt_hfile,LXIOCCAT_SYS,LXIOCFN_SYS_CALL
                 ,&sysc,plen,&plen
                 ,0,dlen,&dlen);
  if(!rc)
  {
   lxapi_sysstate=sysc.sysstate;
   if((sysc.retflags&LX_SYSCALL_RETFLAG_EXITPROCESS))
   {
    DosExit(EXIT_PROCESS,0);
   }
  }
  return sysc.result;
 }
 return -ENOENT;
}

//---------------------------------- syscall1 ----------------------------------
LXAPISYSCRET LXAPIENTRY syscall1(int num
                                 ,unsigned long a1)
{
 if(lxapi_rt_hfile)
 {
  ULONG plen,dlen,rc;
  SYSCALL_PACKET1(sysc,num,a1);
  plen=sizeof(LXIOCPA_SYS_CALL);
  dlen=0;
  rc=DosDevIOCtl(lxapi_rt_hfile,LXIOCCAT_SYS,LXIOCFN_SYS_CALL
                 ,&sysc,plen,&plen
                 ,0,dlen,&dlen);
  if(!rc)
  {
   lxapi_sysstate=sysc.sysstate;
   if((sysc.retflags&LX_SYSCALL_RETFLAG_EXITPROCESS))
   {
    DosExit(EXIT_PROCESS,0);
   }
  }
  return sysc.result;
 }
 return -ENOENT;
}

//---------------------------------- syscall2 ----------------------------------
LXAPISYSCRET LXAPIENTRY syscall2(int num
                                 ,unsigned long a1
                                 ,unsigned long a2)
{
 if(lxapi_rt_hfile)
 {
  ULONG plen,dlen,rc;
  SYSCALL_PACKET2(sysc,num,a1,a2);
  plen=sizeof(LXIOCPA_SYS_CALL);
  dlen=0;
  rc=DosDevIOCtl(lxapi_rt_hfile,LXIOCCAT_SYS,LXIOCFN_SYS_CALL
                 ,&sysc,plen,&plen
                 ,0,dlen,&dlen);
  if(!rc)
  {
   lxapi_sysstate=sysc.sysstate;
   if((sysc.retflags&LX_SYSCALL_RETFLAG_EXITPROCESS))
   {
    DosExit(EXIT_PROCESS,0);
   }
  }
  return sysc.result;
 }
 return -ENOENT;
}

//---------------------------------- syscall3 ----------------------------------
LXAPISYSCRET LXAPIENTRY syscall3(int num
                                 ,unsigned long a1
                                 ,unsigned long a2
                                 ,unsigned long a3)
{
 if(lxapi_rt_hfile)
 {
  ULONG plen,dlen,rc;
  SYSCALL_PACKET3(sysc,num,a1,a2,a3);
  plen=sizeof(LXIOCPA_SYS_CALL);
  dlen=0;
  rc=DosDevIOCtl(lxapi_rt_hfile,LXIOCCAT_SYS,LXIOCFN_SYS_CALL
                 ,&sysc,plen,&plen
                 ,0,dlen,&dlen);
  if(!rc)
  {
   lxapi_sysstate=sysc.sysstate;
   if((sysc.retflags&LX_SYSCALL_RETFLAG_EXITPROCESS))
   {
    DosExit(EXIT_PROCESS,0);
   }
  }
  return sysc.result;
 }
 return -ENOENT;
}

//---------------------------------- syscall4 ----------------------------------
LXAPISYSCRET LXAPIENTRY syscall4(int num
                                 ,unsigned long a1
                                 ,unsigned long a2
                                 ,unsigned long a3
                                 ,unsigned long a4)
{
 if(lxapi_rt_hfile)
 {
  ULONG plen,dlen,rc;
  SYSCALL_PACKET4(sysc,num,a1,a2,a3,a4);
  plen=sizeof(LXIOCPA_SYS_CALL);
  dlen=0;
  rc=DosDevIOCtl(lxapi_rt_hfile,LXIOCCAT_SYS,LXIOCFN_SYS_CALL
                 ,&sysc,plen,&plen
                 ,0,dlen,&dlen);
  if(!rc)
  {
   lxapi_sysstate=sysc.sysstate;
   if((sysc.retflags&LX_SYSCALL_RETFLAG_EXITPROCESS))
   {
    DosExit(EXIT_PROCESS,0);
   }
  }
  return sysc.result;
 }
 return -ENOENT;
}

//---------------------------------- syscall5 ----------------------------------
LXAPISYSCRET LXAPIENTRY syscall5(int num
                                 ,unsigned long a1
                                 ,unsigned long a2
                                 ,unsigned long a3
                                 ,unsigned long a4
                                 ,unsigned long a5)
{
 if(lxapi_rt_hfile)
 {
  ULONG plen,dlen,rc;
  SYSCALL_PACKET5(sysc,num,a1,a2,a3,a4,a5);
  plen=sizeof(LXIOCPA_SYS_CALL);
  dlen=0;
  rc=DosDevIOCtl(lxapi_rt_hfile,LXIOCCAT_SYS,LXIOCFN_SYS_CALL
                 ,&sysc,plen,&plen
                 ,0,dlen,&dlen);
  if(!rc)
  {
   lxapi_sysstate=sysc.sysstate;
   if((sysc.retflags&LX_SYSCALL_RETFLAG_EXITPROCESS))
   {
    DosExit(EXIT_PROCESS,0);
   }
  }
  return sysc.result;
 }
 return -ENOENT;
}

//---------------------------------- syscall6 ----------------------------------
LXAPISYSCRET LXAPIENTRY syscall6(int num
                                 ,unsigned long a1
                                 ,unsigned long a2
                                 ,unsigned long a3
                                 ,unsigned long a4
                                 ,unsigned long a5
                                 ,unsigned long a6)
{
 if(lxapi_rt_hfile)
 {
  ULONG plen,dlen,rc;
  SYSCALL_PACKET6(sysc,num,a1,a2,a3,a4,a5,a6);
  plen=sizeof(LXIOCPA_SYS_CALL);
  dlen=0;
  rc=DosDevIOCtl(lxapi_rt_hfile,LXIOCCAT_SYS,LXIOCFN_SYS_CALL
                 ,&sysc,plen,&plen
                 ,0,dlen,&dlen);
  if(!rc)
  {
   lxapi_sysstate=sysc.sysstate;
   if((sysc.retflags&LX_SYSCALL_RETFLAG_EXITPROCESS))
   {
    DosExit(EXIT_PROCESS,0);
   }
  }
  return sysc.result;
 }
 return -ENOENT;
}
