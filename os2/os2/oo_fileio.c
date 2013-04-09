/* $Id: oo_fileio.c,v 1.4 2005/04/24 01:45:22 smilcke Exp $ */

/*
 * oao_fileio.c
 * Autor:               Stefan Milcke
 * Erstellt am:         27.04.2004
 * Letzte Aenderung am: 23.04.2005
 *
*/

#define INCL_DOSFILEMGR
#include <lxcommon.h>

#include <linux/mm.h>
#include <linux/string.h>

#include <lxdaemon.h>

#include <lxapi.h>

#include <lxsecure.h>

extern unsigned long lx_use_sec_helpers;

#define LXSIOH_IS_SEC   1
#define LXSIOH_IS_DOS   2
struct lx_sio_handle
{
 struct list_head list;
 unsigned long handle;
 int type;
};

LIST_HEAD(lx_sio_handles);
//------------------------------ LX_si_get_handle ------------------------------
static struct lx_sio_handle* LX_si_get_handle(int handle)
{
 struct list_head* lh;
 struct lx_sio_handle* hp;
 list_for_each(lh,&lx_sio_handles)
 {
  hp=list_entry(lh,struct lx_sio_handle,list);
  if(hp->handle==handle)
   return hp;
 }
 return 0;
}


//--------------------------------- LX_si_open ---------------------------------
int LX_si_open(char* sFileName,unsigned long* pHandle
               ,unsigned long ulOpenFlag,unsigned long ulOpenMode)
{
 unsigned long handle=0;
 int rc;
 unsigned long ulAction;
 struct lx_sio_handle* hp=kmalloc(sizeof(struct lx_sio_handle),GFP_KERNEL);
 if(!hp)
  return -ENOMEM;
 if(lx_use_sec_helpers)
  rc=SecHlpOpen(sFileName,&handle,ulOpenFlag,ulOpenMode);
 else
  rc=LX_DOSOPEN(sFileName,&handle,&ulAction,0,FILE_NORMAL,ulOpenFlag,ulOpenMode,0);
 if(!rc)
 {
  hp->handle=handle;
  hp->type=lx_use_sec_helpers ? LXSIOH_IS_SEC : LXSIOH_IS_DOS;
  list_add(&hp->list,&lx_sio_handles);
  *pHandle=handle;
 }
 else
 {
  kfree(hp);
  handle=-ENOENT;
 }
 return rc;
}

//-------------------------------- LX_si_close ---------------------------------
int LX_si_close(unsigned long handle)
{
 int rc;
 struct lx_sio_handle* hp=LX_si_get_handle(handle);
 if(!hp)
  return -ENOENT;
 if(hp->type==LXSIOH_IS_SEC)
  rc=SecHlpClose(handle);
 else
  rc=LX_DOSCLOSE(handle);
 list_del(&hp->list);
 kfree(hp);
 return rc;
}

//--------------------------------- LX_si_read ---------------------------------
int LX_si_read(unsigned long handle,unsigned long* pcbBytes,void* pBuffer)
{
 int rc;
 struct lx_sio_handle* hp=LX_si_get_handle(handle);
 if(!hp)
  return -ENOENT;
 if(hp->type==LXSIOH_IS_SEC)
  rc=SecHlpRead(handle,pcbBytes,pBuffer,0,-1);
 else
  rc=LX_DOSREAD(handle,pBuffer,*pcbBytes,pcbBytes);
 return rc;
}

//-------------------------------- LX_si_linein --------------------------------
char* LX_si_linein(unsigned long handle,unsigned long* pcbBytes,char* pBuffer)
{
 unsigned long b=1;
 char c;
 int nb=0;
 int rc=0;
 char* p=pBuffer;
 struct lx_sio_handle* hp=LX_si_get_handle(handle);
 if(!hp)
  return 0;
 memset(p,0,*pcbBytes);
 while(0==rc && nb<*pcbBytes)
 {
  rc=LX_si_read(handle,&b,&c);
  if(rc)
   break;
  if(b<=0)
   break;
  if(c==0x0a)
  {
   *p=(char)0;
   if(nb>0)
   {
    p--;
    if(*p==0x0d)
    {
     *p=(char)0;
     nb--;
    }
   }
   break;
  }
  *p++=c;
  nb+=b;
 }
 *pcbBytes=nb;
 if(rc || (!b && !nb))
  return 0;
 return pBuffer;
}

//-------------------------------- LX_write_log --------------------------------
ULONG LX_write_log(char* logData)
{
 ULONG rc;
 if(lx_use_sec_helpers)
 {
  ULONG ulActual;
  ULONG bufsize=strlen(logData);
  ULONG handle=0;
  rc=SecHlpOpen(lx_log_file_name,&handle
                , OPEN_ACTION_CREATE_IF_NEW
                 |OPEN_ACTION_OPEN_IF_EXISTS
                , OPEN_FLAGS_WRITE_THROUGH
                 |OPEN_FLAGS_FAIL_ON_ERROR
                 |OPEN_FLAGS_NO_CACHE
                 |OPEN_FLAGS_RANDOMSEQUENTIAL
                 |OPEN_SHARE_DENYNONE
                 |OPEN_ACCESS_READWRITE);
  if(handle)
  {
   rc=SecHlpChgFilePtr(handle,0,SEEK_END,&ulActual);
   if(!rc)
   {
    rc=SecHlpWrite(handle,&bufsize,logData,0,-1);
    SecHlpClose(handle);
   }
  }
 }
 else
  rc=LX_DMN_LOG(logData);
 return rc;
}
