/* $Id: lxlcio.c,v 1.13 2005/07/01 23:49:23 smilcke Exp $ */

/*
 * lxlcio.c
 * Autor:               Stefan Milcke
 * Erstellt am:         10.04.2005
 * Letzte Aenderung am: 02.07.2005
 *
*/

#define INCL_DOS
#define LXFH_MAP
#include <lxlibc.h>

#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <stdarg.h>
#include <share.h>
#include <sys/stat.h>

#include <lxlibcwcalls.h>

#include <linux/err.h>
#include <linux/unistd.h>
#include <lxlist.h>

#include <lxlibcsysc.h>

#define LXMAXPATH    (300)

//------------------------------- LXAPI_os_open --------------------------------
LXWCALLLINKAGE int LXWCATTRINLINE
  LXAPI_os_open(char* pathname,int oflag,int omode)
{
 int rc;
 __asm__ __volatile__
 (
  "pushl %%ebx\n\t"
  "subl  $8,%%esp\n\t"
  "call  _open\n\t"
  "addl  $12,%%esp\n\t"
  :"=a" ((int)rc)
  :"a" ((unsigned long)pathname), "d" ((int)oflag), "b" ((int)omode)
  :"ecx", "esi", "edi", "memory"
 );
 return rc;
}

//------------------------------- LXAPI_os_sopen -------------------------------
LXWCALLLINKAGE int LXWCATTRINLINE
  LXAPI_os_sopen(char* pathname,int oflag,int sflag,int omode)
{
 int rc;
 __asm__ __volatile__
 (
  "pushl %%ebx\n\t"
  "subl  $12,%%esp\n\t"
  "call  __sopen\n\t"
  "addl  $16,%%esp\n\t"
  :"=a" ((int)rc)
  :"a" ((unsigned long)pathname), "d" ((int)oflag), "c" ((int)sflag), "b" ((int)omode)
  :"esi", "edi", "memory"
 );
 return rc;
}

//------------------------------- LXAPI_os_close -------------------------------
LXWCALLLINKAGE int LXWCATTRINLINE
  LXAPI_os_close(int handle)
{
 int rc;
 __asm__ __volatile__
 (
  "call  _close\n\t"
  :"=a" ((int)rc)
  :"a" ((int) handle)
  :"ecx","edx","memory"
 );
 return rc;
}

//------------------------------- LXAPI_os_creat -------------------------------
LXWCALLLINKAGE int LXWCATTRINLINE
  LXAPI_os_creat(char* pathname,int omode)
{
 int rc;
 __asm__ __volatile__
 (
  "call  _creat\n\t"
  :"=a" ((int)rc)
  :"a" ((unsigned long)pathname), "d" ((int)omode)
  :"ecx", "esi", "edi", "memory"
 );
 return rc;
}

//------------------------------- LXAPI_os_read --------------------------------
LXWCALLLINKAGE ssize_t LXWCATTRINLINE
  LXAPI_os_read(int handle,void* buffer,size_t buflen)
{
 ssize_t rc;
 __asm__ __volatile__
 (
  "call  _read\n\t"
  :"=a" ((int)rc)
  :"a" ((int)handle), "d" ((unsigned long)buffer), "c" ((unsigned long)buflen)
  :"esi", "edi", "memory"
 );
 return rc;
}

//------------------------------- LXAPI_os_write -------------------------------
LXWCALLLINKAGE int LXWCATTRINLINE
  LXAPI_os_write(int handle,__const__ void* buffer,size_t buflen)
{
 int rc;
 __asm__ __volatile__
 (
  "call  _write\n\t"
  :"=a" ((int)rc)
  :"a" ((int)handle), "d" ((unsigned long)buffer), "c" ((unsigned long)buflen)
  :"esi", "edi", "memory"
 );
 return rc;
}

//------------------------------- LXAPI_os_lseek -------------------------------
LXWCALLLINKAGE unsigned long LXWCATTRINLINE
  LXAPI_os_lseek(int handle,unsigned long offset,int origin)
{
 unsigned long rc;
 __asm__ __volatile__
 (
  "call  _lseek\n\t"
  :"=a" ((unsigned long)rc)
  :"a" ((int)handle), "d" ((unsigned long)offset), "c" ((unsigned long)origin)
  :"esi", "edi", "memory"
 );
 return rc;
}

//-------------------------------- LXAPI_os_dup --------------------------------
LXWCALLLINKAGE unsigned long LXWCATTRINLINE
  LXAPI_os_dup(int handle)
{
 int rc;
 __asm__ __volatile__
 (
  "call  _dup\n\t"
  :"=a" ((int)rc)
  :"a" ((int) handle)
  :"ecx","edx","memory"
 );
 return rc;
}

//------------------------------- LXAPI_os_dup2 --------------------------------
LXWCALLLINKAGE int LXWCATTRINLINE
  LXAPI_os_dup2(int handle1,int handle2)
{
 int rc;
 __asm__ __volatile__
 (
  "call  _dup2\n\t"
  :"=a" ((int)rc)
  :"a" ((int)handle1), "d" ((int)handle2)
  :"ecx", "esi", "edi", "memory"
 );
 return rc;
}


//------------------------------ maybe_unix_file -------------------------------
int maybe_unix_file(__const__ char* name)
{
 if(strchr(name,'\\'))
  return 0;
 if(strlen(name)>1 && name[1]==':')
  return 0;
 return 1;
}

//-------------------------------- LXAPI_vsopen --------------------------------
static int
  LXAPIENTRY LXAPI_vsopen(__const__ char* pathname,int oflag,int sflag,int omode)
{
 va_list va;
 int h=-1;
 int type;
 char *p;
 p=malloc(LXMAXPATH);
 if(!p)
  return -1;
 strncpy(p,pathname,LXMAXPATH);
 if(maybe_unix_file(p))
 {
  type=LXFH_TYPE_LX;
  h=SYSCALL3(__NR_open,p,oflag,omode);
  if(h<0)
  {
   if(h!=-EPERM)
   {
    type=LXFH_TYPE_OS;
    if(sflag==SH_DENYNO)
     h=LXAPI_os_open(p,LXAPI_convert_oflag_to_os(oflag)
                      ,LXAPI_convert_omode_to_os(omode));
    else
     h=LXAPI_os_sopen(p,LXAPI_convert_oflag_to_os(oflag)
                       ,sflag
                       ,LXAPI_convert_omode_to_os(omode));
   }
   else
   {
    errno=h;   // Note: Negative values indicates a linux error
    h=-1;
   }
  }
 }
 else
 {
  type=LXFH_TYPE_OS;
  if(sflag==SH_DENYNO)
   h=LXAPI_os_open(p,LXAPI_convert_oflag_to_os(oflag)
                    ,LXAPI_convert_omode_to_os(omode));
  else
   h=LXAPI_os_sopen(p,LXAPI_convert_oflag_to_os(oflag)
                     ,sflag
                     ,LXAPI_convert_omode_to_os(omode));
 }
out:
 free(p);
 if(h>=0)
  return (LXAPI_register_filehandle(h,type));
 return h;
}

//--------------------------------- LXAPI_open ---------------------------------
int LXAPIENTRY LXAPI_open(__const__ char* pathname,int oflag,...)
{
 va_list va;
 int omode=0;
 if((oflag&O_CREAT))
 {
  va_start(va,oflag);
  omode=va_arg(va,int);
  va_end(va);
 }
 return LXAPI_vsopen(pathname,oflag,SH_DENYNO,omode);
}

//-------------------------------- LXAPI_sopen ---------------------------------
int LXAPIENTRY LXAPI_sopen(__const__ char* pathname,int oflag,int shflag,...)
{
 va_list va;
 int omode;
 va_start(va,shflag);
 omode=va_arg(va,int);
 va_end(va);
 return LXAPI_vsopen(pathname,oflag,shflag,omode);
}

//-------------------------------- LXAPI_creat ---------------------------------
int LXAPIENTRY LXAPI_creat(__const__ char* pathname,mode_t modet)
{
 char* p;
 int type;
 int h=-1;
 int omode=(int)modet;
 p=malloc(LXMAXPATH);
 if(!p)
  return -1;
 strncpy(p,pathname,LXMAXPATH);
 if(maybe_unix_file(p))
 {
  type=LXFH_TYPE_LX;
  h=SYSCALL2(__NR_creat,p,omode);
  if(h<0)
  {
   if(h!=-EPERM)
   {
    type=LXFH_TYPE_OS;
    h=LXAPI_os_creat(p,LXAPI_convert_omode_to_os(omode));
   }
   else
   {
    errno=h;
    h=-1;
   }
  }
 }
 else
 {
  type=LXFH_TYPE_OS;
  h=LXAPI_os_creat(p,LXAPI_convert_omode_to_os(omode));
 }
 free(p);
 if(h>=0)
  return (LXAPI_register_filehandle(h,type));
 return h;
}

//-------------------------------- LXAPI_close ---------------------------------
int LXAPIENTRY LXAPI_close(int handle)
{
 int rc=-1;
 struct lx_filehandle* fhp;
 fhp=LXAPI_get_filehandle_struct(handle);
 if(fhp)
 {
  if(fhp->type==LXFH_TYPE_LX)
   rc=SYSCALL1(__NR_close,fhp->real_handle);
  else
   rc=LXAPI_os_close(fhp->real_handle);
  LXAPI_deregister_filehandle(handle);
 }
 return rc;
}

//--------------------------------- LXAPI_read ---------------------------------
ssize_t LXAPIENTRY LXAPI_read(int handle,void* buffer,size_t buflen)
{
 ssize_t rc=0;
 struct lx_filehandle* fhp;
 fhp=LXAPI_get_filehandle_struct(handle);
 if(fhp)
 {
  if(fhp->type==LXFH_TYPE_LX)
   rc=SYSCALL3(__NR_read,fhp->real_handle,buffer,buflen);
  else
   rc=LXAPI_os_read(fhp->real_handle,buffer,buflen);
 }
 return rc;
}

//-------------------------------- LXAPI_write ---------------------------------
int LXAPIENTRY LXAPI_write(int handle,__const__ void* buffer,size_t buflen)
{
 int rc=0;
 struct lx_filehandle* fhp;
 fhp=LXAPI_get_filehandle_struct(handle);
 if(fhp)
 {
  if(fhp->type==LXFH_TYPE_LX)
   rc=SYSCALL3(__NR_write,fhp->real_handle,buffer,buflen);
  else
   rc=LXAPI_os_write(fhp->real_handle,buffer,buflen);
 }
 return rc;
}

//-------------------------------- LXAPI_lseek ---------------------------------
off_t LXAPIENTRY LXAPI_lseek(int handle,off_t offset,int origin)
{
 off_t rc=0;
 struct lx_filehandle* fhp;
 fhp=LXAPI_get_filehandle_struct(handle);
 if(fhp)
 {
  if(fhp->type==LXFH_TYPE_LX)
   rc=SYSCALL3(__NR_lseek,fhp->real_handle,offset,origin);
  else
   rc=LXAPI_os_lseek(fhp->real_handle,(unsigned long)offset,origin);
 }
 return rc;
}

//-------------------------------- LXAPI_access --------------------------------
int LXAPIENTRY LXAPI_access(__const__ char* name,int mode)
{
 return SYSCALL2(__NR_access,name,mode);
}

//--------------------------------- LXAPI_dup ----------------------------------
int LXAPIENTRY LXAPI_dup(int handle)
{
 int h=0;
 int type;
 struct lx_filehandle* fhp;
 fhp=LXAPI_get_filehandle_struct(handle);
 if(fhp)
 {
  if(fhp->type==LXFH_TYPE_LX)
   h=SYSCALL1(__NR_dup,fhp->real_handle);
  else
   h=LXAPI_os_dup(fhp->real_handle);
  if(h>=0)
   return (LXAPI_register_filehandle(h,fhp->type));
 }
 return -1;
}

//--------------------------------- LXAPI_dup2 ---------------------------------
int LXAPIENTRY LXAPI_dup2(int handle1,int handle2)
{
 int h=0;
 int type;
 struct lx_filehandle* fhp;
 if(LXAPI_get_filehandle_struct(handle2))
  return -1;
 fhp=LXAPI_get_filehandle_struct(handle1);
 if(fhp)
 {
  if(fhp->type==LXFH_TYPE_LX)
   h=SYSCALL2(__NR_dup2,fhp->real_handle,handle2);
  else
   h=LXAPI_os_dup2(fhp->real_handle,handle2);
  if(h>=0)
   return (LXAPI_register_real_filehandle(handle2,handle2,fhp->type));
 }
 return -1;
}
