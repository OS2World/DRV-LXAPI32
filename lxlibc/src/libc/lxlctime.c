/* $Id: lxlctime.c,v 1.3 2005/07/01 23:49:24 smilcke Exp $ */

/*
 * lxlctime.c
 * Autor:               Stefan Milcke
 * Erstellt am:         16.06.2005
 * Letzte Aenderung am: 02.07.2005
 *
*/

#define INCL_DOS
#define LXFH_MAP
#include <lxlibc.h>

#include <linux/unistd.h>
#include <time.h>
#include <sys/times.h>
#include <utime.h>
#include <sys/timeb.h>

#include <lxlibcsysc.h>

//------------------------------------ time ------------------------------------
time_t time(time_t* pt)
{
 time_t t;
 SYSCALL1(__NR_time,&t);
 if(pt)
  *pt=t;
 return t;
}

//----------------------------------- utime ------------------------------------
int utime(const char* name,const struct utimbuf* times)
{
 return SYSCALL2(__NR_utime,name,times);
}

//----------------------------------- ftime ------------------------------------
void ftime(struct timeb* ti)
{
 SYSCALL1(__NR_ftime,ti);
}

//----------------------------------- times ------------------------------------
long times(struct tms* tms)
{
 return SYSCALL1(__NR_times,tms);
}

//----------------------------------- stime ------------------------------------
int stime(time_t* tptr)
{
 return SYSCALL1(__NR_stime,tptr);
}
