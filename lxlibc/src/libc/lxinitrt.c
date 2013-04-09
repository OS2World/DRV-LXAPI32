/* $Id: lxinitrt.c,v 1.12 2005/07/23 21:15:16 smilcke Exp $ */

/*
 * lxinitrt.c
 * Autor:               Stefan Milcke
 * Erstellt am:         14.03.2005
 * Letzte Aenderung am: 22.07.2005
 *
*/

#define INCL_DOS
#define LXFH_MAP
#include <lxlibc.h>

#include <stdlib.h>

//---------------------------- LXAPI_query_sysstate ----------------------------
LXAPISYSCRET LXAPIENTRY LXAPI_query_sysstate(ULONG flags)
{
 if(flags)
 {
  if(lxapi_rt_hfile)
  {
   ULONG plen,dlen,rc;
   LXIOCPA_UTL_QUERY_SYSTEM_STATE pa;
   plen=sizeof(LXIOCPA_UTL_QUERY_SYSTEM_STATE);
   dlen=0;
   rc=DosDevIOCtl(lxapi_rt_hfile,LXIOCCAT_UTL,LXIOCFN_UTL_QUERY_SYSTEM_STATE
                  ,&pa,plen,&plen
                  ,0,dlen,&dlen);
   if(!rc)
    lxapi_sysstate=pa.sysstate;
  }
 }
 return lxapi_sysstate;
}

//---------------------------- LXAPI_os2path_to_lx -----------------------------
int LXAPIENTRY LXAPI_os2path_to_lx(const char* os2path,char* lxpath)
{
 ULONG plen,dlen,rc;
 LXIOCPA_UTL_OS2PATHTOLX pa;
 plen=sizeof(LXIOCPA_UTL_OS2PATHTOLX);
 strcpy(pa.os2path,os2path);
 strcpy(pa.lxpath,"");
 dlen=0;
 rc=DosDevIOCtl(lxapi_rt_hfile,LXIOCCAT_UTL,LXIOCFN_UTL_OS2PATHTOLX
                ,&pa,plen,&plen
                ,0,dlen,&dlen);
 if(!rc)
 {
  strcpy(lxpath,pa.lxpath);
  return pa.rc;
 }
 return rc;
}

//------------------------------- init_work_dir --------------------------------
static void init_work_dir(void)
{
 ULONG clen=LX_MAXPATH-3;
 ULONG cdisk;
 ULONG dmap;
 char curpath[LX_MAXPATH];
 char lxpath[LX_MAXPATH];
 curpath[1]=':';
 curpath[2]='\\';
 curpath[3]=(char)0;
 if(!DosQueryCurrentDisk(&cdisk,&dmap))
 {
  curpath[0]=(char)(cdisk-1)+'A';
  if(!DosQueryCurrentDir(0,&(curpath[3]),&clen))
  {
   if(!LXAPI_os2path_to_lx(curpath,lxpath))
   {
    chdir(lxpath);
    chdir(lxpath);
   }
  }
 }
}

//----------------------------- LXAPI_init_runtime -----------------------------
LXAPISYSCRET LXAPIENTRY LXAPI_init_runtime(void)
{
 unsigned long rc;
 ULONG action;
 DosSleep(10);
 rc=DosOpen("LXAPIUT$",&lxapi_rt_hfile,&action,0L,0,1,OPEN_SHARE_DENYNONE,0);
 if(!rc)
 {
  LXAPI_query_sysstate(1);
  init_work_dir();
 }
 return rc;
}

//----------------------------- LXAPI_term_runtime -----------------------------
LXAPISYSCRET LXAPIENTRY LXAPI_term_runtime(void)
{
 unsigned long rc=0;
 if(lxapi_rt_hfile)
 {
  rc=DosClose(lxapi_rt_hfile);
  lxapi_rt_hfile=0;
 }
 return rc;
}

//------------------------- LXAPI_init_process_started -------------------------
LXAPISYSCRET LXAPIENTRY LXAPI_init_process_started(void)
{
 unsigned long rc=-1;
 if(!(lxapi_sysstate&LXSYSSTATE_INIT_STARTED))
 {
  if(!(lxapi_sysstate&LXSYSSTATE_NOT_OPERABLE))
  {
   unsigned long plen=0,dlen=0;
   rc=DosDevIOCtl(lxapi_rt_hfile,LXIOCCAT_UTL,LXIOCFN_UTL_INIT_STARTED
                  ,0,plen,&plen
                  ,0,dlen,&dlen);
   LXAPI_query_sysstate(1);
  }
 }
 return rc;
}

//------------------------ LXAPI_init_process_finished -------------------------
LXAPISYSCRET LXAPIENTRY LXAPI_init_process_finished(unsigned long flags)
{
 unsigned long rc=-1;
 if((lxapi_sysstate&LXSYSSTATE_INIT_STARTED))
 {
  if(!(lxapi_sysstate&LXSYSSTATE_INIT_FINISHED))
  {
   unsigned long plen,dlen;
   LXIOCPA_UTL_INIT_FINISHED finished;
   finished.exit_code=0;
   finished.exit_flags=flags;
   plen=sizeof(LXIOCPA_UTL_INIT_FINISHED);
   dlen=0;
   rc=DosDevIOCtl(lxapi_rt_hfile,LXIOCCAT_UTL,LXIOCFN_UTL_INIT_FINISHED
                  ,&finished,plen,&plen
                  ,0,dlen,&dlen);
   LXAPI_query_sysstate(1);
  }
 }
 return rc;
}

//----------------------------------- __main -----------------------------------
void __main(void)
{
 asm("int $3;");
}

//--------------------------------- LXAPI_exit ---------------------------------
void LXAPIENTRY LXAPI_exit(int rc)
{
 LXAPI_term_runtime();
 __asm__ __volatile__("call exit\n\t": :"a"((unsigned long)rc) );
}
