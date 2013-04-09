/* $Id: lxlcsys.c,v 1.3 2005/07/01 23:49:24 smilcke Exp $ */

/*
 * lxlcsys.c
 * Autor:               Stefan Milcke
 * Erstellt am:         16.06.2005
 * Letzte Aenderung am: 02.07.2005
 *
*/

#define INCL_DOS
#define LXFH_MAP
#include <lxlibc.h>

#include <linux/unistd.h>

#include <lxlibcsysc.h>

//------------------------------------ sync ------------------------------------
void sync(void)
{
 SYSCALL0(__NR_sync);
}

//-------------------------------- LXAPI_umask ---------------------------------
int LXAPI_umask(int pmode)
{
 return SYSCALL1(__NR_umask,pmode);
}

//------------------------------- LXAPI_truncate -------------------------------
int LXAPI_truncate(__const__ char* name,off_t offset)
{
 return SYSCALL2(__NR_truncate,name,offset);
}

//----------------------------------- lchown -----------------------------------
int lchown(__const__ char* name,uid_t uid,gid_t gid)
{
 int rc;
 rc=SYSCALL3(__NR_lchown,name,uid,gid);
 return rc;
}

//----------------------------------- fchown -----------------------------------
int fchown(int fp,uid_t uid,gid_t gid)
{
 int rc;
 rc=SYSCALL3(__NR_fchown,fp,uid,gid);
 return rc;
}

//----------------------------------- getpid -----------------------------------
pid_t getpid(void)
{
 return (pid_t)SYSCALL0(__NR_getpid);
}

//----------------------------------- getuid -----------------------------------
uid_t getuid(void)
{
 return (uid_t)SYSCALL0(__NR_getuid);
}

//----------------------------------- getgid -----------------------------------
gid_t getgid(void)
{
 return (gid_t)SYSCALL0(__NR_getgid);
}

//----------------------------------- setuid -----------------------------------
int setuid(uid_t uid)
{
 return SYSCALL1(__NR_setuid,uid);
}

//----------------------------------- setgid -----------------------------------
int setgid(gid_t gid)
{
 return SYSCALL1(__NR_setgid,gid);
}

//---------------------------------- setpgid -----------------------------------
int setpgid(pid_t pid,pid_t gid)
{
 return SYSCALL2(__NR_setpgid,pid,gid);
}

//---------------------------------- geteuid -----------------------------------
uid_t geteuid(void)
{
 return (uid_t)SYSCALL0(__NR_geteuid);
}

//---------------------------------- getegid -----------------------------------
gid_t getegid(void)
{
 return (gid_t)SYSCALL0(__NR_getegid);
}

//---------------------------------- getppid -----------------------------------
pid_t getppid(void)
{
 return (pid_t)SYSCALL0(__NR_getppid);
}

//---------------------------------- getpgrp -----------------------------------
pid_t getpgrp(void)
{
 return (pid_t)SYSCALL0(__NR_getpgrp);
}

//----------------------------------- setsid -----------------------------------
pid_t setsid(void)
{
 return (pid_t)SYSCALL0(__NR_setsid);
}

//---------------------------------- setreuid ----------------------------------
int setreuid(uid_t p1,uid_t p2)
{
 return SYSCALL2(__NR_setreuid,p1,p2);
}

//---------------------------------- setregid ----------------------------------
int setregid(gid_t p1,gid_t p2)
{
 return SYSCALL2(__NR_setregid,p1,p2);
}

//-------------------------------- setpriority ---------------------------------
int setpriority(int which,int who,int niceval)
{
 return SYSCALL3(__NR_setpriority,which,who,niceval);
}

//-------------------------------- getpriority ---------------------------------
int getpriority(int which,int who)
{
 return SYSCALL2(__NR_getpriority,which,who);
}

//----------------------------------- pause ------------------------------------
int pause(void)
{
 return SYSCALL0(__NR_pause);
}

//------------------------------------ nice ------------------------------------
int nice(int increment)
{
 return SYSCALL1(__NR_nice,increment);
}
