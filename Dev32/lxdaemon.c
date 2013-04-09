/* $Id: lxdaemon.c,v 1.36 2006/01/05 23:48:13 smilcke Exp $ */

/*
 * lxdaemon.c
 * Autor:               Stefan Milcke
 * Erstellt am:         12.04.2004
 * Letzte Aenderung am: 31.12.2005
 *
*/

#define INCL_DOSERRORS
#include <lxcommon.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/module.h>

#include <DEVRP.H>
#include <lxdaemon.h>
#include <lxapioctl.h>


#define LX_DAEMON_STATE_UNINITIALIZED  0
#define LX_DAEMON_STATE_COMING         1
#define LX_DAEMON_STATE_RUNNING        2
#define LX_DAEMON_STATE_GOING          3
#define LX_DAEMON_STATE_GONE           4

static int delayed_log=1;
MODULE_PARM(delayed_log,"i");

volatile ULONG lx_daemon_state=0;
volatile unsigned short lx_daemon_pid=0;
volatile unsigned long lx_daemon_cmdhandle=0;

LIST_HEAD(lx_waiting_command_list);
LIST_HEAD(lx_done_command_list);
spinlock_t lx_cmd_lock=SPIN_LOCK_UNLOCKED;
struct lx_command_list_entry
{
 struct list_head list;
 LXIOCPA_DMN_CMDPARMPACKET parmPacket;
};

#define LXDMN_PARMPACKETLENGTH \
 (sizeof(struct lx_command_list_entry)+sizeof(LXIOCPA_DMN_CMDPARMPACKET))

#define LXDMN_DP_FROM_CMDLISTENRY(p) \
 ((void*)((unsigned long)(p)+LXDMN_PARMPACKETLENGTH))


/*******************************************************************************/
/* Called from daemon IOCTL                                                    */
/*******************************************************************************/
//-------------------------- LX_dmn_add_done_command ---------------------------
ULONG LX_dmn_add_done_command(PLXIOCPA_DMN_CMDPARMPACKET pParm,PVOID pData)
{
 unsigned long f;
 unsigned long sz=LXDMN_PARMPACKETLENGTH;
 struct lx_command_list_entry* cmd;
 if(!(pParm->cmdBlockID))
  sz+=pParm->cbCmdDataLen;
 cmd=kmalloc(sz,GFP_KERNEL);
 if(!cmd)
  return -ENOMEM;
 memcpy(&cmd->parmPacket,pParm,sizeof(LXIOCPA_DMN_CMDPARMPACKET));
 if(pParm->cmdBlockID)
  memcpy((PVOID)pParm->cmdBlockID,pData,pParm->cbCmdDataLen);
 spin_lock_irqsave(&lx_cmd_lock,f);
 list_add_tail(&cmd->list,&lx_done_command_list);
 {
  spin_unlock_irqrestore(&lx_cmd_lock,f);
  LX_RunOn((ULONG)pParm->cmdBlockID);
 }
 return 0;
}

//---------------------------- LX_get_next_command -----------------------------
struct lx_command_list_entry* LX_get_next_command(void)
{
 unsigned long f;
 struct lx_command_list_entry* pe=0;
 while(!pe)
 {
  if(list_empty(&lx_waiting_command_list))
   LX_WaitOn((unsigned long)&lx_waiting_command_list,-1,0);
  spin_lock_irqsave(&lx_cmd_lock,f);
  if(!list_empty(&lx_waiting_command_list))
  {
   pe=(struct lx_command_list_entry*)(lx_waiting_command_list.next);
   list_del(&pe->list);
  }
  spin_unlock_irqrestore(&lx_cmd_lock,f);
 }
 return pe;
}

//-------------------------------- LX_ioctl_dmn --------------------------------
ULONG LX_ioctl_dmn(struct LX_RP* prp)
{
 ULONG rc=0;
 ULONG cpuflags;
 switch(prp->RPBODY.RPIOCTL.Function)
 {
  case LXIOCFN_DMN_CMDGET:
   {
    PARMPACKET(pa,PLXIOCPA_DMN_CMDPARMPACKET,prp,VERIFY_READWRITE);
    DATAPACKET(dp,PVOID,prp,VERIFY_READWRITE);
    if(pa && dp)
    {
     struct lx_command_list_entry* pe;
     if(!lx_daemon_pid)
     {
      lx_daemon_pid=lx_current_pid;
      if(lx_daemon_state<LX_DAEMON_STATE_RUNNING)
       lx_daemon_state=LX_DAEMON_STATE_RUNNING;
      LX_RunOn((unsigned long)&lx_daemon_state);
     }
     pe=LX_get_next_command();
     if(pe)
     {
      if(pe->parmPacket.cbCmdDataLen>pa->cbMaxDataLen)
      {
       spin_lock_irqsave(&lx_cmd_lock,cpuflags);
       list_add(&pe->list,&lx_waiting_command_list);
       spin_unlock_irqrestore(&lx_cmd_lock,cpuflags);
       pa->cmd=LX_DMN_CMD_INC_DATAPACKETLEN;
       pa->hcmd=0;
       pa->cmdBlockID=0;
      }
      else
      {
       pe->parmPacket.rc=pa->rc;
       pe->parmPacket.cbMaxDataLen=pa->cbMaxDataLen;
       memcpy(pa,&(pe->parmPacket)
              ,sizeof(LXIOCPA_DMN_CMDPARMPACKET));
       if(pe->parmPacket.cmdBlockID)
        memcpy(dp,(PVOID)pe->parmPacket.cmdBlockID,pe->parmPacket.cbCmdDataLen);
       else
        memcpy(dp,LXDMN_DP_FROM_CMDLISTENRY(pe),pe->parmPacket.cbCmdDataLen);
       kfree(pe);
      }
     }
     else
      rc=RPERR_COMMAND;
    }
    else
     rc=RPERR_COMMAND;
   }
   break;
  case LXIOCFN_DMN_CMDSETRESULTS:
   {
    PARMPACKET(pa,PLXIOCPA_DMN_CMDPARMPACKET,prp,VERIFY_READWRITE);
    DATAPACKET(dp,PVOID,prp,VERIFY_READWRITE);
    if(pa && dp)
    {
     if(pa->cmdBlockID)
      LX_dmn_add_done_command(pa,dp);
    }
   }
   break;
  default:
   rc=RPERR_COMMAND;
 }
 return rc | RPDONE;
}

/*******************************************************************************/
/* Called from within the driver                                               */
/*******************************************************************************/
//----------------------------- LX_dmn_get_result ------------------------------
ULONG LX_dmn_get_result(PLXIOCPA_DMN_CMDPARMPACKET pParm,PVOID pData)
{
 ULONG cpuflags;
 ULONG done=0;
 struct list_head* lh;
 while(!done)
 {
  LX_WaitOn(pParm->cmdBlockID,1000,0);
  spin_lock_irqsave(&lx_cmd_lock,cpuflags);
  list_for_each(lh,&lx_done_command_list)
  {
   struct lx_command_list_entry* pe=list_entry(lh,struct lx_command_list_entry
                                               ,list);
   if(pe->parmPacket.hcmd==pParm->hcmd)
   {
    done=1;
    list_del(&pe->list);
    memcpy(pParm,&pe->parmPacket,sizeof(LXIOCPA_DMN_CMDPARMPACKET));
    kfree(pe);
    spin_unlock_irqrestore(&lx_cmd_lock,cpuflags);
    return pParm->rc;
   }
  }
  spin_unlock_irqrestore(&lx_cmd_lock,cpuflags);
 }
 return pParm->rc;
}

//----------------------------- LX_dmn_add_command -----------------------------
ULONG LX_dmn_add_command(ULONG ulCmd,ULONG waitflag
                         ,PLXIOCPA_DMN_CMDPARMPACKET pParm
                         ,PVOID pData
                         ,ULONG cbDataLen)
{
 if(lx_daemon_pid)
 {
  unsigned long f;
  unsigned long sz=LXDMN_PARMPACKETLENGTH;
  struct lx_command_list_entry* cmd;
  if(0==waitflag)
   sz+=cbDataLen;
  cmd=kmalloc(sz,GFP_KERNEL);
  if(!cmd)
   return -ENOMEM;
  pParm->rc=0;
  pParm->cmd=ulCmd;
  pParm->cbCmdDataLen=cbDataLen;
  if(waitflag)
   pParm->cmdBlockID=(unsigned long)pData;
  else
  {
   pParm->cmdBlockID=0;
   memcpy(LXDMN_DP_FROM_CMDLISTENRY(cmd),pData,pParm->cbCmdDataLen);
  }
  memcpy(&cmd->parmPacket,pParm,sizeof(LXIOCPA_DMN_CMDPARMPACKET));
  spin_lock_irqsave(&lx_cmd_lock,f);
  lx_daemon_cmdhandle++;
  if(0==lx_daemon_cmdhandle)
   lx_daemon_cmdhandle++;
  pParm->hcmd=lx_daemon_cmdhandle;
  cmd->parmPacket.hcmd=lx_daemon_cmdhandle;
  list_add_tail(&cmd->list,&lx_waiting_command_list);
  {
   spin_unlock_irqrestore(&lx_cmd_lock,f);
   LX_RunOn((ULONG)&lx_waiting_command_list);
  }
  if(pParm->cmdBlockID)
   return LX_dmn_get_result(pParm,pData);
 }
 return 0;
}

//----------------------------- LX_wait_for_daemon -----------------------------
ULONG LX_wait_for_daemon(void)
{
 ULONG rc=-ENOENT;
 if(!(LX_DAEMON_STATE_RUNNING==lx_daemon_state))
 {
  if(lx_daemon_state<LX_DAEMON_STATE_RUNNING)
  {
   int c=100;
   while(c>0 && LX_DAEMON_STATE_RUNNING>lx_daemon_state)
   {
    LX_WaitOn((ULONG)&lx_daemon_state,100,0);
    c--;
   }
  }
 }
 if(LX_DAEMON_STATE_RUNNING==lx_daemon_state)
  rc=0;
 return rc;
}

//------------------------------ LX_daemon_closed ------------------------------
ULONG LX_daemon_closed(void)
{
 ULONG f;
 struct lx_command_list_entry* pe=0;
 struct list_head* lh;
 lx_daemon_pid=0;
 lx_daemon_state=LX_DAEMON_STATE_GOING;
 LX_RunOn((ULONG)&lx_daemon_pid);
 spin_lock_irqsave(&lx_cmd_lock,f);
 while(!list_empty(&lx_waiting_command_list))
 {
  pe=(struct lx_command_list_entry*)(lx_waiting_command_list.next);
  list_del(&pe->list);
  if(pe->parmPacket.cmdBlockID)
   list_add_tail(&pe->list,&lx_done_command_list);
  else
   kfree(pe);
 }
 list_for_each(lh,&lx_done_command_list)
 {
  pe=list_entry(lh,struct lx_command_list_entry,list);
  pe->parmPacket.rc=ERROR_NO_CHILDREN;
  LX_RunOn(pe->parmPacket.cmdBlockID);
 }
 spin_unlock_irqrestore(&lx_cmd_lock,f);
 return 0;
}

//----------------------------- LX_daemon_shutdown -----------------------------
ULONG LX_daemon_shutdown(void)
{
 ULONG c=20;
 LXIOCPA_DMN_CMDPARMPACKET p;
 p.cmd=LX_DMN_CMD_SHUTDOWN;
 p.cbCmdDataLen=0;
 LX_dmn_add_command(LX_DMN_CMD_SHUTDOWN,1,&p,0,0);
 while(c && lx_daemon_pid)
 {
  LX_WaitOn((ULONG)&lx_daemon_pid,100,0);
  c--;
 }
 if(lx_daemon_pid)
  return LX_daemon_closed();
 return 0;
}

extern char lx_drv_homepath[];
//--------------------------- LX_InitComplete_Daemon ---------------------------
void LX_InitComplete_Daemon(void)
{
/*
 LXIOCPA_DMN_CMDPARMPACKET p;
 char fn[LX_MAXPATH];
 char failname[LX_MAXPATH];
 RESULTCODES res;
 strcpy(fn,lx_drv_homepath);
 strcat(fn,"LXAPID.EXE");
 if(!LX_DOSEXECPGM(failname,LX_MAXPATH,EXEC_ASYNC,0,0,0,0,&res,fn))
  LX_dmn_add_command(LX_DMN_CMD_RESTART,1,&p,fn,LX_MAXPATH);
*/
}

//------------------------------ LX_DMN_CHAR_CMD -------------------------------
static ULONG LX_DMN_CHAR_CMD(char* pData,ULONG cmd)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 LX_dmn_add_command(cmd,(0==delayed_log),&p,pData,strlen(pData)+1);
 return 0;
}

//--------------------------------- LX_DMN_LOG ---------------------------------
ULONG LX_DMN_LOG(char* logData)
{
 return LX_DMN_CHAR_CMD(logData,LX_DMN_CMD_LOG);
}

//----------------------------- LX_DMN_SETLOGNAME ------------------------------
ULONG LX_DMN_SETLOGNAME(char* logFileName)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 return LX_dmn_add_command(LX_DMN_CMD_SET_LOGNAME,1,&p,logFileName
                           ,strlen(logFileName)+1);
}

//-------------------------------- LX_EADELETE ---------------------------------
ULONG LX_EADELETE(char* fName,char* eaName)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXTWONAMESTRUCT tn;
 strncpy(tn.name1,fName,LX_MAXPATH);
 strncpy(tn.name2,eaName,LX_MAXPATH);
#if 1
 LX_dmn_add_command(LX_DMN_CMD_EADELETE,0,&p,&tn,sizeof(LXTWONAMESTRUCT));
 return 0;
#else
 LX_dmn_add_command(LX_DMN_CMD_EADELETE,1,&p,&tn,sizeof(LXTWONAMESTRUCT));
 return p.rc;
#endif
}

//------------------------------ LX_EAREADSTRING -------------------------------
ULONG LX_EAREADSTRING(char* fName,char* eaName,void* buffer,PULONG pBufLen)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 PLXEAREADSTRINGSTRUCT ep;
 ULONG sz=sizeof(LXEAREADSTRINGSTRUCT)+*pBufLen;
 ep=kmalloc(sz,GFP_KERNEL);
 if(ep)
 {
  char *eaVal=(char*)(((ULONG)ep)+sizeof(LXEAREADSTRINGSTRUCT));
  strncpy(ep->fileName,fName,LX_MAXPATH);
  ep->eaValLen=*pBufLen;
  strncpy(ep->eaName,eaName,LX_MAXPATH);
  LX_dmn_add_command(LX_DMN_CMD_EAREADSTRING,1,&p,ep,sz);
  if(!p.rc)
  {
   strncpy(buffer,eaVal,*pBufLen);
   *pBufLen=ep->eaValLen;
  }
  kfree(ep);
 }
 else
  p.rc=-ENOMEM;
 return p.rc;
}

//------------------------------ LX_EAWRITESTRING ------------------------------
ULONG LX_EAWRITESTRING(char* fName,char* eaName,void* buffer,PULONG pBufLen)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 PLXEAWRITESTRINGSTRUCT ep;
 ULONG sz=sizeof(LXEAWRITESTRINGSTRUCT)+(*pBufLen);
 ep=kmalloc(sz,GFP_KERNEL);
 if(ep)
 {
  char *eaVal=(char*)(((ULONG)ep)+sizeof(LXEAWRITESTRINGSTRUCT));
  strncpy(ep->fileName,fName,LX_MAXPATH);
  ep->eaValLen=*pBufLen;
  strncpy(ep->eaName,eaName,LX_MAXPATH);
  strncpy(eaVal,buffer,*pBufLen);
#if 1
  LX_dmn_add_command(LX_DMN_CMD_EAWRITESTRING,1,&p,ep,sz);
  kfree(ep);
 }
 else
  return -ENOMEM;
 return 0;
#else
  LX_dmn_add_command(LX_DMN_CMD_EAWRITESTRING,1,&p,ep,sz);
  kfree(ep);
 }
 else
  p.rc=-ENOMEM;
 return p.rc;
#endif
}

//------------------------------- LX_GETFILEINFO -------------------------------
ULONG LX_GETFILEINFO(char* fileName,PLXGETFILEINFOSTRUCT pfs)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 strcpy(pfs->fileName,fileName);
 LX_dmn_add_command(LX_DMN_CMD_GETFILEINFO,1,&p,pfs,sizeof(LXGETFILEINFOSTRUCT));
 return p.rc;
}

//------------------------------ LX_LXAFS_RENAME -------------------------------
ULONG LX_LXAFS_RENAME(char* oldName,char* newName)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXTWONAMESTRUCT tn;
 strncpy(tn.name1,oldName,LX_MAXPATH);
 strncpy(tn.name2,newName,LX_MAXPATH);
 LX_dmn_add_command(LX_DMN_CMD_LXAFSRENAME,1,&p,&tn,sizeof(LXTWONAMESTRUCT));
 return p.rc;
}

/*******************************************************************************/
/* DOS API wrappers                                                            */
/*******************************************************************************/
//--------------------------------- LX_DOSBEEP ---------------------------------
ULONG LX_DOSBEEP(ULONG waitflag,ULONG frequency,ULONG duration)
{
 ULONG rc=0;
 LXDOSBEEPSTRUCT bs;
 LXIOCPA_DMN_CMDPARMPACKET p;
 bs.frequency=frequency;
 bs.duration=duration;
 rc=LX_dmn_add_command(LX_DMN_CMD_DOSBEEP,waitflag,&p,&bs
                       ,sizeof(LXDOSBEEPSTRUCT));
 return rc;
}

//------------------------------- LX_DOSSCANENV --------------------------------
ULONG LX_DOSSCANENV(char* name,char* value)
{
 char pData[2000];
 LXIOCPA_DMN_CMDPARMPACKET p;
 strcpy(pData,name);
 LX_dmn_add_command(LX_DMN_CMD_DOSSCANENV,1,&p,pData,2000);
 if(!p.rc)
  strcpy(value,pData);
 return p.rc;
}

//--------------------------------- LX_DOSMOVE ---------------------------------
ULONG LX_DOSMOVE(char* oldName,char* newName)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXTWONAMESTRUCT tn;
 strncpy(tn.name1,oldName,LX_MAXPATH);
 strncpy(tn.name2,newName,LX_MAXPATH);
 LX_dmn_add_command(LX_DMN_CMD_DOSMOVE,1,&p,&tn,sizeof(LXTWONAMESTRUCT));
 return p.rc;
}

//------------------------------ LX_DOSCREATEDIR -------------------------------
ULONG LX_DOSCREATEDIR(char* dirName,PEAOP2 peaop2)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXDOSCREATEDIRSTRUCT cd;
 strncpy(cd.dirName,dirName,LX_MAXPATH);
// cd.eaop2={0}; // eaop2 not supported yet!!!
 LX_dmn_add_command(LX_DMN_CMD_DOSCREATEDIR,1,&p,&cd,sizeof(LXDOSCREATEDIRSTRUCT));
 return p.rc;
}

//--------------------------------- LX_DOSOPEN ---------------------------------
ULONG LX_DOSOPEN(char* fileName,PHFILE pHf,PULONG pulAction
                 ,ULONG cbFile,ULONG ulAttribute
                 ,ULONG fsOpenFlags,ULONG fsOpenMode
                 ,PEAOP2 peaop2)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXDOSOPENSTRUCT os;
 strncpy(os.fileName,fileName,LX_MAXPATH);
 os.cbFile=cbFile;
 os.ulAttribute=ulAttribute;
 os.fsOpenFlags=fsOpenFlags;
 os.fsOpenMode=fsOpenMode;
// os.eaop2={0}; // eaop2 not supported yet!!!
 LX_dmn_add_command(LX_DMN_CMD_DOSOPEN,1,&p,&os,sizeof(LXDOSOPENSTRUCT));
 if(!p.rc)
 {
  *pHf=os.hFile;
  *pulAction=os.ulAction;
 }
 return p.rc;
}

//------------------------------ LX_DOSFINDFIRST -------------------------------
ULONG LX_DOSFINDFIRST(char* fileSpec,PHDIR pHDir,ULONG flAttribute
                      ,PVOID pfindbuf,ULONG cbBuf,PULONG pcFileNames
                      ,ULONG ulInfoLevel)
{
 ULONG sz=sizeof(LXDOSFINDFIRSTSTRUCT)+cbBuf;
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXDOSFINDFIRSTSTRUCT *pfs;
 pfs=kmalloc(sz,GFP_KERNEL);
 if(pfs)
 {
  strncpy(pfs->fileSpec,fileSpec,LX_MAXPATH);
  memcpy(&(pfs->LXDOSFINDFIRSTUNION.fileFindBuf3),pfindbuf,cbBuf);
  pfs->flAttribute=flAttribute;
  pfs->cbBuf=cbBuf;
  pfs->cFileNames=*pcFileNames;
  pfs->ulInfoLevel=ulInfoLevel;
  pfs->hDir=*pHDir;
  LX_dmn_add_command(LX_DMN_CMD_DOSFINDFIRST,1,&p,pfs,sz);
  if(!p.rc)
  {
   memcpy(pfindbuf,&(pfs->LXDOSFINDFIRSTUNION.fileFindBuf3),cbBuf);
   *pHDir=pfs->hDir;
   *pcFileNames=pfs->cFileNames;
  }
  kfree(pfs);
  return p.rc;
 }
 return -ENOMEM;
}

//------------------------------- LX_DOSFINDNEXT -------------------------------
ULONG LX_DOSFINDNEXT(HDIR hDir,PVOID pfindbuf,ULONG cbBuf,PULONG pcFileNames)
{
 ULONG sz=sizeof(LXDOSFINDFIRSTSTRUCT)+cbBuf;
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXDOSFINDFIRSTSTRUCT *pfs;
 pfs=kmalloc(sz,GFP_KERNEL);
 if(pfs)
 {
  memcpy(&(pfs->LXDOSFINDFIRSTUNION.fileFindBuf3),pfindbuf,cbBuf);
  pfs->hDir=hDir;
  pfs->cbBuf=cbBuf;
  pfs->cFileNames=*pcFileNames;
  LX_dmn_add_command(LX_DMN_CMD_DOSFINDNEXT,1,&p,pfs,sz);
  if(!p.rc)
  {
   memcpy(pfindbuf,&(pfs->LXDOSFINDFIRSTUNION.fileFindBuf3),cbBuf);
   *pcFileNames=pfs->cFileNames;
  }
  kfree(pfs);
  return p.rc;
 }
 return -ENOMEM;
}

//--------------------------- LX_DOSQUERYCURRENTDISK ---------------------------
ULONG LX_DOSQUERYCURRENTDISK(PULONG pDiskNum,PULONG pDriveMap)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXDOSQUERYCURRENTDISKSTRUCT s;
 LX_dmn_add_command(LX_DMN_CMD_DOSQUERYCURRENTDISK,1,&p,&s,sizeof(LXDOSQUERYCURRENTDISKSTRUCT));
 if(!p.rc)
 {
  *pDiskNum=s.diskNum;
  *pDriveMap=s.driveMap;
 }
 return p.rc;
}

//------------------------------ LX_DOSFINDCLOSE -------------------------------
ULONG LX_DOSFINDCLOSE(HDIR hDir)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 p.rc=0;
 LX_dmn_add_command(LX_DMN_CMD_DOSFINDCLOSE,1,&p,&hDir,sizeof(HDIR));
 return p.rc;
}

//-------------------------------- LX_DOSCLOSE ---------------------------------
ULONG LX_DOSCLOSE(HFILE hFile)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 p.rc=0;
 LX_dmn_add_command(LX_DMN_CMD_DOSCLOSE,1,&p,&hFile,sizeof(HFILE));
 return p.rc;
}

//------------------------------ LX_DOSSETFILEPTR ------------------------------
ULONG LX_DOSSETFILEPTR(HFILE hFile,LONG ib,ULONG method,PULONG pibActual)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXDOSSETFILEPTRSTRUCT sp;
 sp.hFile=hFile;
 sp.ib=ib;
 sp.method=method;
 if(pibActual)
 {
  LX_dmn_add_command(LX_DMN_CMD_DOSSETFILEPTR,1,&p,&sp,sizeof(LXDOSSETFILEPTRSTRUCT));
  if(!p.rc)
   *pibActual=sp.ibActual;
  return p.rc;
 }
 LX_dmn_add_command(LX_DMN_CMD_DOSSETFILEPTR,1,&p,&sp,sizeof(LXDOSSETFILEPTRSTRUCT));
 return 0;
}

//--------------------------------- LX_DOSREAD ---------------------------------
ULONG LX_DOSREAD(HFILE hFile,PVOID pBuffer,ULONG cbRead,PULONG pcbActual)
{
 ULONG sz=sizeof(LXDOSREADWRITESTRUCT)+cbRead;
 LXIOCPA_DMN_CMDPARMPACKET p;
 PLXDOSREADWRITESTRUCT prw=kmalloc(sz,GFP_KERNEL);
 if(!prw)
  return -ENOMEM;
 prw->hFile=hFile;
 prw->cbReadWrite=cbRead;
 LX_dmn_add_command(LX_DMN_CMD_DOSREAD,1,&p,prw,sz);
 if(!p.rc)
 {
  memcpy(pBuffer,prw->data,prw->cbActual);
  if(pcbActual)
   *pcbActual=prw->cbActual;
 }
 kfree(prw);
 return p.rc;
}

//-------------------------------- LX_DOSWRITE ---------------------------------
ULONG LX_DOSWRITE(HFILE hFile,PVOID pBuffer,ULONG cbWrite,PULONG pcbActual)
{
 ULONG sz=sizeof(LXDOSREADWRITESTRUCT)+cbWrite;
 LXIOCPA_DMN_CMDPARMPACKET p;
 PLXDOSREADWRITESTRUCT prw=kmalloc(sz,GFP_KERNEL);
 if(!prw)
  return -ENOMEM;
 prw->hFile=hFile;
 prw->cbReadWrite=cbWrite;
 memcpy(prw->data,pBuffer,cbWrite);
 LX_dmn_add_command(LX_DMN_CMD_DOSWRITE,1,&p,prw,sz);
 if(!p.rc)
 if(pcbActual)
 {
  if(!p.rc)
   *pcbActual=prw->cbActual;
  else
   *pcbActual=0;
 }
 kfree(prw);
 return p.rc;
}

//----------------------------- LX_DOSQUERYSYSINFO -----------------------------
ULONG LX_DOSQUERYSYSINFO(ULONG iStart,ULONG iLast,VOID* pBuffer,ULONG cbBuf)
{
 ULONG sz=sizeof(LXDOSQUERYSYSINFOSTRUCT)+cbBuf;
 LXIOCPA_DMN_CMDPARMPACKET p;
 PLXDOSQUERYSYSINFOSTRUCT prw=kmalloc(sz,GFP_KERNEL);
 if(!prw)
  return -ENOMEM;
 prw->iStart=iStart;
 prw->iLast=iLast;
 prw->cbBuf=cbBuf;
 LX_dmn_add_command(LX_DMN_CMD_DOSQUERYSYSINFO,1,&p,prw,sz);
 memcpy(pBuffer,prw,cbBuf);
 kfree(prw);
 return p.rc;
}

//------------------------------- LX_DOSEXECPGM --------------------------------
ULONG LX_DOSEXECPGM(PCHAR pObjname,LONG cbObjname,ULONG execFlag
                    ,PSZ pArg,LONG arglen,PSZ pEnv,LONG envlen
                    ,PRESULTCODES pRes,PSZ pName)
{
 ULONG sz=sizeof(LXDOSEXECPGMSTRUCT)+arglen+envlen+2;
 LXIOCPA_DMN_CMDPARMPACKET p;
 char *parg=0;
 char *penv=0;
 PLXDOSEXECPGMSTRUCT pe=kmalloc(sz,GFP_KERNEL);
 if(!pe)
  return -ENOMEM;
 memset(pe,0,sz);
 strncpy(pe->name,pName,LX_MAXPATH);
 pe->cbObjname=cbObjname;
 pe->execFlag=execFlag;
 if(0!=arglen)
 {
  parg=(char*)(((unsigned long)pe)+sizeof(LXDOSEXECPGMSTRUCT));
  memcpy(parg,pArg,arglen);
  pe->arglen=arglen;
 }
 if(0!=envlen)
 {
  penv=(char*)(((unsigned long)pe)+arglen+sizeof(LXDOSEXECPGMSTRUCT));
  memcpy(penv,pEnv,envlen);
  pe->envlen=envlen;
 }
 LX_dmn_add_command(LX_DMN_CMD_DOSEXECPGM,1,&p,pe,sz);
 if(pObjname && cbObjname)
  memcpy(pObjname,pe->objname,cbObjname);
 memcpy(pRes,&pe->res,sizeof(RESULTCODES));
 kfree(pe);
 return p.rc;
}

//----------------------------- LX_DOSSETFILESIZE ------------------------------
ULONG LX_DOSSETFILESIZE(HFILE hFile,ULONG cbSize)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXDOSSETFILESIZESTRUCT fs;
 fs.hFile=hFile;
 fs.cbSize=cbSize;
 LX_dmn_add_command(LX_DMN_CMD_DOSSETFILESIZE,1,&p,&fs
                    ,sizeof(LXDOSSETFILESIZESTRUCT));
 return p.rc;
}

//---------------------------- LX_DOSQUERYFILEINFO -----------------------------
ULONG LX_DOSQUERYFILEINFO(HFILE hFile,ULONG ulInfoLevel
                          ,VOID* pInfoBuf,ULONG cbInfoBuf)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXDOSQUERYFILEINFOSTRUCT fi;
 fi.hFile=hFile;
 fi.ulInfoLevel=ulInfoLevel;
 fi.cbInfoBuf=cbInfoBuf;
 LX_dmn_add_command(LX_DMN_CMD_DOSQUERYFILEINFO,1,&p,&fi
                    ,sizeof(LXDOSQUERYFILEINFOSTRUCT));
 if(!p.rc)
 {
  memcpy(pInfoBuf,&(fi.filestatus),cbInfoBuf);
 }
 return p.rc;
}

//----------------------------- LX_DOSSETFILEINFO ------------------------------
ULONG LX_DOSSETFILEINFO(HFILE hFile,ULONG ulInfoLevel
                        ,VOID* pInfoBuf,ULONG cbInfoBuf)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXDOSSETFILEINFOSTRUCT fi;
 fi.hFile=hFile;
 fi.ulInfoLevel=ulInfoLevel;
 fi.cbInfoBuf=cbInfoBuf;
 memcpy(&(fi.filestatus),pInfoBuf,cbInfoBuf);
 LX_dmn_add_command(LX_DMN_CMD_DOSSETFILEINFO,1,&p,&fi
                    ,sizeof(LXDOSSETFILEINFOSTRUCT));
 return p.rc;
}

//---------------------------- LX_DOSQUERYPATHINFO -----------------------------
ULONG LX_DOSQUERYPATHINFO(char* name,ULONG ulInfoLevel
                          ,VOID* pInfoBuf,ULONG cbInfoBuf)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXDOSQUERYPATHINFOSTRUCT pi;
 strncpy(pi.pathName,name,LX_MAXPATH);
 pi.ulInfoLevel=ulInfoLevel;
 pi.cbInfoBuf=cbInfoBuf;
 LX_dmn_add_command(LX_DMN_CMD_DOSQUERYPATHINFO,1,&p,&pi
                    ,sizeof(LXDOSQUERYPATHINFOSTRUCT));
 if(!p.rc)
  memcpy(pInfoBuf,&pi.LXDOSQUERYPATHINFOUNION.fileStatus3,cbInfoBuf);
 return p.rc;
}

//----------------------------- LX_DOSSETPATHINFO ------------------------------
ULONG LX_DOSSETPATHINFO(char* name,ULONG ulInfoLevel
                        ,VOID* pInfoBuf,ULONG cbInfoBuf,ULONG flOptions)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXDOSSETPATHINFOSTRUCT pi;
 strncpy(pi.pathName,name,LX_MAXPATH);
 pi.ulInfoLevel=ulInfoLevel;
 pi.cbInfoBuf=cbInfoBuf;
 pi.flOptions=flOptions;
 memcpy(&pi.LXDOSSETPATHINFOUNION.fileStatus3,pInfoBuf,cbInfoBuf);
 LX_dmn_add_command(LX_DMN_CMD_DOSSETPATHINFO,1,&p,&pi
                    ,sizeof(LXDOSSETPATHINFOSTRUCT));
 return p.rc;
}

//---------------------------- LX_DOSENUMATTRIBUTE -----------------------------
ULONG LX_DOSENUMATTRIBUTE(ULONG ulRefType,PVOID vFile,ULONG ulEntry
                          ,PVOID pBuffer,ULONG cbBuf,PULONG pulCount
                          ,ULONG ulInfoLevel)
{
 ULONG sz=sizeof(LXDOSENUMATTRIBUTESTRUCT)+cbBuf;
 LXIOCPA_DMN_CMDPARMPACKET p;
 PLXDOSENUMATTRIBUTESTRUCT pea=0;
 p.rc=-ENOMEM;
 pea=kmalloc(sz,GFP_KERNEL);
 if(pea)
 {
  pea->ulRefType=ulRefType;
  if(pea->ulRefType)
   strcpy(pea->vFile,vFile);
  else
   *((PULONG)pea->vFile)=*((PULONG)vFile);
  pea->ulEntry=ulEntry;
  pea->cbBuf=cbBuf;
  pea->ulCount=*pulCount;
  pea->ulInfoLevel=ulInfoLevel;
  LX_dmn_add_command(LX_DMN_CMD_DOSENUMATTRIBUTE,1,&p,pea,sz);
  if(!p.rc)
  {
   void* pSrc=(void*)(((unsigned long)pea)+sizeof(LXDOSENUMATTRIBUTESTRUCT));
   memcpy(pBuffer,pSrc,cbBuf);
   *pulCount=pea->ulCount;
  }
  kfree(pea);
 }
 return p.rc;
}

//-------------------------------- LX_DOSDELETE --------------------------------
ULONG LX_DOSDELETE(char* name)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXONENAMESTRUCT on;
 strcpy(on.name,name);
 LX_dmn_add_command(LX_DMN_CMD_DOSDELETE,1,&p,&on,sizeof(LXONENAMESTRUCT));
 return p.rc;
}

//------------------------------ LX_DOSDELETEDIR -------------------------------
ULONG LX_DOSDELETEDIR(char* name)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXONENAMESTRUCT on;
 strcpy(on.name,name);
 LX_dmn_add_command(LX_DMN_CMD_DOSDELETEDIR,1,&p,&on,sizeof(LXONENAMESTRUCT));
 return p.rc;
}

//--------------------------------- LX_DOSCOPY ---------------------------------
ULONG LX_DOSCOPY(char* sourceFile,char* targetFile,ULONG option)
{
 LXIOCPA_DMN_CMDPARMPACKET p;
 LXDOSCOPYSTRUCT cs;
 strcpy(cs.sourceFile,sourceFile);
 strcpy(cs.targetFile,targetFile);
 cs.option=option;
 LX_dmn_add_command(LX_DMN_CMD_DOSCOPY,1,&p,&cs,sizeof(LXDOSCOPYSTRUCT));
 return p.rc;
}
