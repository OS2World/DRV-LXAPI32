/* $Id: lxsh.c,v 1.4 2005/03/15 23:38:21 smilcke Exp $ */

/*
 * lxsh.c
 * Autor:               Stefan Milcke
 * Erstellt am:         14.02.2005
 * Letzte Aenderung am: 07.03.2005
 *
*/

//#include <lxcommon.h>

#define INCL_DOS
#include <os2.h>
#include <string.h>

//#include <lxlibc.h>

HFILE lxapi_rt_hfile=0;

//----------------------------- LXAPI_init_runtime -----------------------------
unsigned long LXAPI_init_runtime(void)
{
 unsigned long rc;
 ULONG action;
 DosSleep(50);
 rc=DosOpen("LXAPIUT$",&lxapi_rt_hfile,&action,0L,0,1,OPEN_SHARE_DENYNONE,0);
 return rc;
}

//----------------------------- LXAPI_term_runtime -----------------------------
unsigned long LXAPI_term_runtime(void)
{
 unsigned long rc=0;
 if(lxapi_rt_hfile)
 {
  rc=DosClose(lxapi_rt_hfile);
 }
 return rc;
}

//------------------------------------ main ------------------------------------
int main(int argc,void* argv[])
{
 LXAPI_init_runtime();
 LXAPI_term_runtime();
 return 0;
}
