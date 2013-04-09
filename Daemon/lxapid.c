/* $Id: lxapid.c,v 1.26 2005/06/13 22:16:32 smilcke Exp $ */

/*
 * lxapid.c
 * Autor:               Stefan Milcke
 * Erstellt am:         03.11.2003
 * Letzte Aenderung am: 13.06.2005
 *
*/

#define INCL_32
#define INCL_DOS
//#define INCL_DOSSEMAPHORES
#include <os2.h>

#include <string.h>
#include <stdlib.h>

#include <lxdaemon.h>

#include "eah.h"

#define INITAL_DATA_LEN (4096)
#define INCR_DATA_LEN   (4096)

int dorestart=0;
unsigned long dataLen=INITAL_DATA_LEN;
void *pData=0;
LXIOCPA_DMN_CMDPARMPACKET param;
char logfilename[LX_MAXPATH]={0};

extern int lxmap_dosapi_error(int err);

//---------------------------------- writeLog ----------------------------------
static unsigned long writeLog(char* pData)
{
 if(logfilename[0]!=0)
 {
  HFILE hFile;
  ULONG action;
  ULONG actual;
  param.rc=DosOpen(logfilename,&hFile,&action,0
                   ,FILE_NORMAL
                   ,OPEN_ACTION_CREATE_IF_NEW
                    | OPEN_ACTION_OPEN_IF_EXISTS
                   ,OPEN_FLAGS_WRITE_THROUGH
                    | OPEN_FLAGS_FAIL_ON_ERROR
                    | OPEN_FLAGS_NO_CACHE
                    | OPEN_FLAGS_RANDOMSEQUENTIAL
                    | OPEN_SHARE_DENYNONE
                    | OPEN_ACCESS_READWRITE
                    ,0);
  if(!param.rc)
  {
   param.rc=DosSetFilePtr(hFile,0,FILE_END,&actual);
   if(!param.rc)
   {
    if(param.cbCmdDataLen>1)
     param.rc=DosWrite(hFile,pData,param.cbCmdDataLen-1,&actual);
    else
     param.rc=DosWrite(hFile,pData,param.cbCmdDataLen,&actual);
   }
   DosClose(hFile);
  }
 }
 return param.rc;
}

//-------------------------- increaseDataPacketLength --------------------------
static unsigned long increaseDataPacketLength(void)
{
 void* pOldData=pData;
 dataLen+=INCR_DATA_LEN;
 param.cbMaxDataLen=dataLen;
 param.rc=DosAllocMem(&pData,dataLen,PAG_COMMIT | PAG_READ | PAG_WRITE);
 if(!param.rc)
 {
  if(pOldData)
   DosFreeMem(pOldData);
 }
 else
 {
  pData=pOldData;
  dataLen-=INCR_DATA_LEN;
 }
 return param.rc;
}

//--------------------------------- setLogFile ---------------------------------
static unsigned long setLogFile(char* logfile)
{
 char s1[300];
 char s2[300];
 int i;
 int s;
 strcpy(s1,logfile);
 strcpy(s2,logfile);
 s=strlen(logfile)-1;
 for(i=9;i>0;i--)
 {
  s1[s]=(char)((i-1)+'0');
  s2[s]=(char)((i)+'0');
  DosCopy(s1,s2,DCPY_EXISTING);
 }
 s2[s]='0';
 DosCopy(logfile,s2,DCPY_EXISTING);
 DosDelete(logfile);
 strcpy(logfilename,logfile);
 return 0;
}

//------------------------ CMD_IncreaseDataPacketLength ------------------------
void CMD_IncreaseDataPacketLength(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                                  ,void* pData)
{
 increaseDataPacketLength();
}

//------------------------------- CMD_SetLogFile -------------------------------
void CMD_SetLogFile(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam,void* pData)
{
 setLogFile(pData);
}

//-------------------------------- CMD_WriteLog --------------------------------
void CMD_WriteLog(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam,void* pData)
{
 writeLog(pData);
}


#define CMD_FN(name) void name(HFILE,LXIOCPA_DMN_CMDPARMPACKET*,void*)
CMD_FN(CMD_DosDelete);
CMD_FN(CMD_DosMove);
CMD_FN(CMD_DosDeleteDir);
CMD_FN(CMD_DosSetFileInfo);
CMD_FN(CMD_DosSetPathInfo);
CMD_FN(CMD_DosExecPgm);
CMD_FN(CMD_DosCopy);
CMD_FN(CMD_DosEnumAttribute);
CMD_FN(CMD_DosQueryPathInfo);
CMD_FN(CMD_DosScanEnv);
CMD_FN(CMD_DosSetFilePtr);
CMD_FN(CMD_DosClose);
CMD_FN(CMD_DosFindClose);
CMD_FN(CMD_DosFindFirst);
CMD_FN(CMD_DosFindNext);
CMD_FN(CMD_DosCreateDir);
CMD_FN(CMD_DosSetFileSize);
CMD_FN(CMD_DosOpen);
CMD_FN(CMD_DosQueryCurrentDisk);
CMD_FN(CMD_DosQueryFileInfo);
CMD_FN(CMD_DosRead);
CMD_FN(CMD_DosWrite);
CMD_FN(CMD_DosBeep);
CMD_FN(CMD_EAReadString);
CMD_FN(CMD_EAWriteString);
CMD_FN(CMD_EADelete);
CMD_FN(CMD_GetFileInfo);
CMD_FN(CMD_DosQuerySysInfo);
CMD_FN(CMD_LXAFS_Rename);

struct lx_cmd
{
 unsigned long cmd;
 void (*fn)(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam,void* pData);
} lx_cmds[]=
{
 /* DOS Wrappers */
 {LX_DMN_CMD_DOSDELETE,             CMD_DosDelete},
 {LX_DMN_CMD_DOSMOVE,               CMD_DosMove},
 {LX_DMN_CMD_DOSDELETEDIR,          CMD_DosDeleteDir},
 {LX_DMN_CMD_DOSSETFILEINFO,        CMD_DosSetFileInfo},
 {LX_DMN_CMD_DOSSETPATHINFO,        CMD_DosSetPathInfo},
 {LX_DMN_CMD_DOSEXECPGM,            CMD_DosExecPgm},
 {LX_DMN_CMD_DOSCOPY,               CMD_DosCopy},
 {LX_DMN_CMD_DOSENUMATTRIBUTE,      CMD_DosEnumAttribute},
 {LX_DMN_CMD_DOSQUERYPATHINFO,      CMD_DosQueryPathInfo},
 {LX_DMN_CMD_DOSSCANENV,            CMD_DosScanEnv},
 {LX_DMN_CMD_DOSSETFILEPTR,         CMD_DosSetFilePtr},
 {LX_DMN_CMD_DOSCLOSE,              CMD_DosClose},
 {LX_DMN_CMD_DOSFINDCLOSE,          CMD_DosFindClose},
 {LX_DMN_CMD_DOSFINDFIRST,          CMD_DosFindFirst},
 {LX_DMN_CMD_DOSFINDNEXT,           CMD_DosFindNext},
 {LX_DMN_CMD_DOSCREATEDIR,          CMD_DosCreateDir},
 {LX_DMN_CMD_DOSSETFILESIZE,        CMD_DosSetFileSize},
 {LX_DMN_CMD_DOSOPEN,               CMD_DosOpen},
 {LX_DMN_CMD_DOSQUERYCURRENTDISK,   CMD_DosQueryCurrentDisk},
 {LX_DMN_CMD_DOSQUERYFILEINFO,      CMD_DosQueryFileInfo},
 {LX_DMN_CMD_DOSREAD,               CMD_DosRead},
 {LX_DMN_CMD_DOSWRITE,              CMD_DosWrite},
 {LX_DMN_CMD_DOSBEEP,               CMD_DosBeep},

 /* EA functions */
 {LX_DMN_CMD_EAREADSTRING,          CMD_EAReadString},
 {LX_DMN_CMD_EAWRITESTRING,         CMD_EAWriteString},
 {LX_DMN_CMD_EADELETE,              CMD_EADelete},

 /* High level functions */
 {LX_DMN_CMD_GETFILEINFO,           CMD_GetFileInfo},
 {LX_DMN_CMD_LXAFSRENAME,           CMD_LXAFS_Rename},

 /* Misc functions */
 {LX_DMN_CMD_DOSQUERYSYSINFO,       CMD_DosQuerySysInfo},
 {LX_DMN_CMD_INC_DATAPACKETLEN,     CMD_IncreaseDataPacketLength},
 {LX_DMN_CMD_LOG,                   CMD_WriteLog},
 {LX_DMN_CMD_SET_LOGNAME,           CMD_SetLogFile},
 {0,0},
};


//----------------------------- executeNewCommand ------------------------------
static void
  executeNewCommand(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam,void* pData)
{
 unsigned long paramLen,dataLen;
 int i;
 for(i=0;lx_cmds[i].fn!=0;i++)
 {
  if(pParam->cmd==lx_cmds[i].cmd)
  {
   lx_cmds[i].fn(hFile,pParam,pData);
//   pParam->rc=lxmap_dosapi_error(pParam->rc);
   return;
  }
 }
 writeLog("Unknown command\n");
 pParam->rc=1;
// pParam->rc=lxmap_dosapi_error(1);
}

//-------------------------------- commandLoop ---------------------------------
static int commandLoop(HFILE hFile)
{
 unsigned long rc=0;
 unsigned long paramLen,dataLen;
 int dorestart=0;
 param.rc=0;
 param.cmd=0;
 while(param.cmd!=LX_DMN_CMD_SHUTDOWN && 0==rc)
 {
  param.rc=0;
  param.cmd=0;
  paramLen=sizeof(LXIOCPA_DMN_CMDPARMPACKET);
  dataLen=param.cbMaxDataLen;
  rc=DosDevIOCtl(hFile,LXIOCCAT_DMN,LXIOCFN_DMN_CMDGET
                 ,&param,paramLen,&paramLen
                 ,pData, dataLen ,&dataLen);
  if(param.cmd==LX_DMN_CMD_RESTART)
  {
   dorestart=1;
   rc=0;
   break;
  }
  executeNewCommand(hFile,&param,pData);
  paramLen=sizeof(LXIOCPA_DMN_CMDPARMPACKET);
  dataLen=param.cbMaxDataLen;
  rc=DosDevIOCtl(hFile,LXIOCCAT_DMN,LXIOCFN_DMN_CMDSETRESULTS
                 ,&param,paramLen,&paramLen
                 ,pData, dataLen ,&dataLen);
  if(rc)
  {
//   DosBeep(440,50);
//   DosBeep(880,50);
   rc=0;
  }
 }
 param.rc=0;
 paramLen=sizeof(LXIOCPA_DMN_CMDPARMPACKET);
 dataLen=param.cbMaxDataLen;
 DosDevIOCtl(hFile,LXIOCCAT_DMN,LXIOCFN_DMN_CMDSETRESULTS
             ,&param,paramLen,&paramLen
             ,pData, dataLen ,&dataLen);
 return (int)rc;
}

//------------------------------------ main ------------------------------------
int main(int argc,char* argv[])
{
 ULONG rc=0;
 ULONG action;
 HFILE hfile_lxapi=0;
 int c=100;
 DosSleep(100);
 if(increaseDataPacketLength())
  return 1;
 while(!hfile_lxapi && c>0)
 {
  rc=DosOpen("LXAPID$"
             ,&hfile_lxapi
             ,&action
             ,0L
             ,0
             ,1
             ,OPEN_SHARE_DENYNONE
             ,0);
  DosSleep(250);
 }
 if(!rc)
 {
  rc=commandLoop(hfile_lxapi);
  DosClose(hfile_lxapi);
 }
 else
 {
  DosBeep(880,500);
  DosBeep(440,500);
 }
 return rc;
}

