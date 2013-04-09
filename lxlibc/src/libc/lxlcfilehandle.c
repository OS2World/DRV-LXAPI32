/* $Id: lxlcfilehandle.c,v 1.3 2005/07/01 23:49:23 smilcke Exp $ */

/*
 * lxlcfilehandle.c
 * Autor:               Stefan Milcke
 * Erstellt am:         17.04.2005
 * Letzte Aenderung am: 02.07.2005
 *
*/

#define INCL_DOS
#define LXFH_MAP
#include <lxlibc.h>

#include <stdlib.h>

#include <lxlist.h>

LIST_HEAD(lx_filehandles);

//----------------------- LXAPI_register_real_filehandle -----------------------
int LXAPIENTRY LXAPI_register_real_filehandle(int real_handle,int mapped_handle
                                              ,int type)
{
 struct lx_filehandle* p=malloc(sizeof(struct lx_filehandle));
 if(!p)
  return -1;
 memset(p,0,sizeof(struct lx_filehandle));
 p->type=type;
 p->mapped_handle=mapped_handle;
 p->real_handle=real_handle;
 list_add(&p->list,&lx_filehandles);
 return 0;
}

//------------------------- LXAPI_register_filehandle --------------------------
int LXAPIENTRY LXAPI_register_filehandle(int handle,int type)
{
 struct list_head* lh;
 int mapped_handle=2;
 int found=1;
 while(1==found)
 {
  found=0;
  list_for_each(lh,&lx_filehandles)
  {
   struct lx_filehandle* tmp=list_entry(lh,struct lx_filehandle,list);
   if(tmp->mapped_handle==mapped_handle)
   {
    mapped_handle++;
    found=1;
    break;
   }
  }
 }
 if(!LXAPI_register_real_filehandle(handle,mapped_handle,type))
  return mapped_handle;
 else
  return -1;
}

//------------------------ LXAPI_deregister_filehandle -------------------------
int LXAPIENTRY LXAPI_deregister_filehandle(int handle)
{
 struct lx_filehandle* fhp=LXAPI_get_filehandle_struct(handle);
 if(fhp)
 {
  list_del(&fhp->list);
  free(fhp);
 }
 return 0;
}

//------------------------ LXAPI_get_filehandle_struct -------------------------
struct lx_filehandle* LXAPIENTRY LXAPI_get_filehandle_struct(int handle)
{
 struct lx_filehandle* fhp;
 struct list_head* lh;
 list_for_each(lh,&lx_filehandles)
 {
  fhp=list_entry(lh,struct lx_filehandle,list);
  if(fhp->mapped_handle==handle)
   return fhp;
 }
 return 0;
}
