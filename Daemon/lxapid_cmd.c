/* $Id: lxapid_cmd.c,v 1.4 2006/01/05 23:48:12 smilcke Exp $ */

/*
 * lxapid_cmd.c
 * Autor:               Stefan Milcke
 * Erstellt am:         13.06.2005
 * Letzte Aenderung am: 29.12.2005
 *
*/

#define INCL_32
#define INCL_DOS
//#define INCL_DOSSEMAPHORES
#include <os2.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <lxdaemon.h>

#include "eah.h"

//------------------------------ simple_d_sprintf ------------------------------
static void simple_d_sprintf(char* buf,char* fmt,int d)
{
 int i,j,c;
 char tmp[20]={0};
 _itoa(d,tmp,10);
 i=0;
 j=0;
 while(fmt[i])
 {
  if(fmt[i]=='%' && fmt[i+1]=='d')
  {
   c=0;
   while(tmp[c])
   {
    buf[j]=tmp[c];
    j++;
    c++;
   }
   j--;
   i++;
  }
  else
   buf[j]=fmt[i];
  i++;
  j++;
 }
 buf[j]=(char)0;
}

//----------------------------- eahWriteStringEA_F -----------------------------
static unsigned long eahWriteStringEA_F(char* fName,char* eaname
                                        ,char* eaval,unsigned long* vallen)
{
 unsigned long rc;
 HFILE hF;
 ULONG action;
 rc=DosOpen(fName,&hF,&action,0,FILE_NORMAL
            ,OPEN_ACTION_FAIL_IF_NEW
             | OPEN_ACTION_OPEN_IF_EXISTS
            ,OPEN_FLAGS_WRITE_THROUGH
             | OPEN_FLAGS_FAIL_ON_ERROR
             | OPEN_FLAGS_NO_CACHE
             | OPEN_FLAGS_RANDOMSEQUENTIAL
             | OPEN_SHARE_DENYNONE
             | OPEN_ACCESS_READWRITE
            ,0);
 if(rc==0 && hF!=0)
 {
  rc=eahWriteStringEA(hF,eaname,eaval);
  DosClose(hF);
 }
 return rc;
}

//--------------------------- lxafs_correct_linkinfo ---------------------------
static int lxafs_correct_linkinfo(char* newname,char* oldname
                                  ,char* newdir,char* olddir)
{
 int err=0;
 char eaname[LX_MAXPATH];
 char eaval[LX_MAXPATH];
 char tmp[LX_MAXPATH];
 ULONG vallen;
 int i;
 strcpy(eaname,LXEAN_LINKBYCOUNT);
 strcpy(eaval,"0");
 vallen=LX_MAXPATH;
 if(!eahReadStringEA(newname,eaname,eaval,&vallen))
 {
  i=atoi(eaval);
  while(i)
  {
   simple_d_sprintf(eaname,LXEAN_LINKEDBY,i);
   vallen=LX_MAXPATH;
   if(!eahReadStringEA(newname,eaname,eaval,&vallen))
   {
    if(!strncmp(eaval,olddir,strlen(olddir)))
    {
     strncpy(tmp,&eaval[strlen(olddir)],LX_MAXPATH);
     strncpy(eaval,newdir,LX_MAXPATH-strlen(tmp));
     strcat(eaval,tmp);
     vallen=strlen(eaval)+1;
     eahWriteStringEA_F(newname,eaname,eaval,&vallen);
    }
   }
   i--;
  }
 }
 strcpy(eaname,LXEAN_LINKEDTO);
 vallen=LX_MAXPATH;
 if(!eahReadStringEA(newname,eaname,eaval,&vallen))
 {
  if(!strncmp(eaval,olddir,strlen(olddir)))
  {
   strncpy(tmp,&eaval[strlen(olddir)],LX_MAXPATH);
   strncpy(eaval,newdir,LX_MAXPATH-strlen(tmp));
   strcat(eaval,tmp);
   vallen=strlen(eaval)+1;
   eahWriteStringEA_F(newname,eaname,eaval,&vallen);
  }
 }
 return err;
}

//------------------------- lxafs_linkinfo_correction --------------------------
static int lxafs_linkinfo_correction(char* newname,char* oldname)
{
 int err=0;
 HDIR hDir=HDIR_CREATE;
 FILEFINDBUF3 fb;
 char filespec[LX_MAXPATH];
 char newoldname[LX_MAXPATH];
 char newnewname[LX_MAXPATH];
 ULONG sz;
 ULONG cFileNames=1;
 sz=sizeof(FILEFINDBUF3);
 strncpy(filespec,newname,LX_MAXPATH-3);
 strcat(filespec,"/*");
 if(!DosFindFirst(filespec,&hDir
                  ,FILE_ARCHIVED|FILE_DIRECTORY|FILE_SYSTEM|FILE_HIDDEN|FILE_READONLY
                  ,&fb,sz,&cFileNames,FIL_STANDARD))
 {
  while(cFileNames)
  {
   if(strcmp(fb.achName,"..") && strcmp(fb.achName,"."))
   {
    strncpy(newoldname,oldname,LX_MAXPATH);
    strncpy(newnewname,newname,LX_MAXPATH);
    strcat(newoldname,"/");
    strcat(newnewname,"/");
    strcat(newoldname,fb.achName);
    strcat(newnewname,fb.achName);
    if((fb.attrFile&FILE_DIRECTORY))
     lxafs_linkinfo_correction(newnewname,newoldname);
    lxafs_correct_linkinfo(newnewname,newoldname,newname,oldname);
   }
   if(DosFindNext(hDir,&fb,sz,&cFileNames) || cFileNames==0)
   {
    cFileNames=0;
    break;
   }
  }
 }
 if(hDir && hDir!=HDIR_CREATE)
  DosFindClose(hDir);
 return err;
}

//------------------------------ CMD_LXAFS_Rename ------------------------------
void CMD_LXAFS_Rename(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                      ,PLXTWONAMESTRUCT tn)
{
 char* oldname=tn->name1;
 char* newname=tn->name2;
 pParam->rc=DosMove(oldname,newname);
 if(!pParam->rc)
 {
  FILESTATUS4 fs4;
  pParam->rc=DosQueryPathInfo(newname,FIL_QUERYEASIZE,&fs4,sizeof(FILESTATUS4));
  if(!pParam->rc)
   if(fs4.attrFile&FILE_DIRECTORY)
    pParam->rc=lxafs_linkinfo_correction(newname,oldname);
 }
}

//------------------------------ CMD_GetFileInfo -------------------------------
void CMD_GetFileInfo(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                     ,PLXGETFILEINFOSTRUCT pf)
{
 ULONG rc;
 ULONG blen;
 CHAR filemask[CCHMAXPATH];
 CHAR tmp[LX_MAXPATH];
 pf->num_subdirs=0;
 pf->num_files=0;
 pf->mode=0;
 pf->uid=0;
 pf->gid=0;
 pf->lc_count=0;
 pf->link_broken=0;
 pf->flags=0;
 memset(pf->linkName,0,LX_MAXPATH);
 // Get Info
 rc=DosQueryPathInfo(pf->fileName,FIL_QUERYEASIZE,&pf->fs4,sizeof(FILESTATUS4));
 if(!rc)
 {
  // If it is a directory, get number of subdirs and files
  if(pf->fs4.attrFile&FILE_DIRECTORY)
  {
   HDIR hDir=HDIR_CREATE;
   FILEFINDBUF3 fb;
   ULONG ulFindCount=1;
   strcpy(filemask,pf->fileName);
   strcat(filemask,"/*");
   rc=DosFindFirst(filemask,&hDir
                   ,FILE_ARCHIVED|FILE_DIRECTORY|FILE_SYSTEM|FILE_HIDDEN|FILE_READONLY
                   ,&fb,sizeof(FILEFINDBUF3)
                   ,&ulFindCount
                   ,FIL_STANDARD);
   while(!rc && ulFindCount>0)
   {
    if(fb.attrFile&FILE_DIRECTORY)
     pf->num_subdirs++;
    else
     pf->num_files++;
    rc=DosFindNext(hDir,&fb,sizeof(FILEFINDBUF3),&ulFindCount);
   }
   DosFindClose(hDir);
   rc=0;
  }
  // If it contains EA's, try to get LX_LC (link count)
  if(pf->fs4.cbList>4)
  {
   blen=CCHMAXPATH;
   eahReadStringEA(pf->fileName,LXEAN_LINKEDTO,pf->linkName,&blen);
   blen=CCHMAXPATH;
   strcpy(filemask,"0");
   eahReadStringEA(pf->fileName,LXEAN_LINKBYCOUNT,filemask,&blen);
   pf->lc_count=atoi(filemask);
   // Check, if link target exists
   if(strcmp(pf->fileName,pf->linkName))
   {
    FILESTATUS4 fs4;
    if(DosQueryPathInfo(pf->linkName,FIL_QUERYEASIZE,&fs4,sizeof(FILESTATUS4)))
    {
     pf->link_broken=1;
     memset(pf->linkName,0,LX_MAXPATH);
    }
   }
   else
    memset(pf->linkName,0,LX_MAXPATH);
  }
  blen=LX_MAXPATH;
  if(!eahReadStringEA(pf->fileName,LXEAN_MODE,tmp,&blen))
  {
   pf->mode=atoi(tmp);
   pf->flags|=LXGETFILEINFOFLAG_MODE;
  }
  blen=LX_MAXPATH;
  if(!eahReadStringEA(pf->fileName,LXEAN_UID,tmp,&blen))
  {
   pf->uid=atoi(tmp);
   pf->flags|=LXGETFILEINFOFLAG_UID;
  }
  blen=LX_MAXPATH;
  if(!eahReadStringEA(pf->fileName,LXEAN_GID,tmp,&blen))
  {
   pf->gid=atoi(tmp);
   pf->flags|=LXGETFILEINFOFLAG_GID;
  }
  // SYMLINK !!!
 }
 pParam->rc=rc;
}

//------------------------------- CMD_DosExecPgm -------------------------------
void CMD_DosExecPgm(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                    ,PLXDOSEXECPGMSTRUCT p)
{
 char *pArg=0;
 char *pEnv=0;
 if(p->arglen)
 {
  pArg=(char*)(((unsigned long)p)+sizeof(LXDOSEXECPGMSTRUCT));
 }
 if(p->envlen)
  pEnv=(char*)(((unsigned long)p)+p->arglen+sizeof(LXDOSEXECPGMSTRUCT));
 pParam->rc=DosExecPgm(p->objname,p->cbObjname,p->execFlag,pArg,pEnv
                      ,&(p->res),p->name);
}

//------------------------------ CMD_DosCreateDir ------------------------------
void CMD_DosCreateDir(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                      ,PLXDOSCREATEDIRSTRUCT p)
{
 pParam->rc=DosCreateDir(p->dirName,NULL);
}

//-------------------------------- CMD_DosMove ---------------------------------
void CMD_DosMove(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                 ,PLXTWONAMESTRUCT tn)
{
 pParam->rc=DosMove(tn->name1,tn->name2);
}

//-------------------------------- CMD_DosOpen ---------------------------------
void CMD_DosOpen(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                 ,PLXDOSOPENSTRUCT p)
{
 pParam->rc=DosOpen(p->fileName,&p->hFile,&p->ulAction,p->cbFile
                    ,p->ulAttribute,p->fsOpenFlags,p->fsOpenMode
                    ,NULL);
}

//-------------------------- CMD_DosQueryCurrentDisk ---------------------------
void CMD_DosQueryCurrentDisk(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                             ,PLXDOSQUERYCURRENTDISKSTRUCT p)
{
 pParam->rc=DosQueryCurrentDisk(&p->diskNum,&p->driveMap);
}

//------------------------------ CMD_DosFindFirst ------------------------------
void CMD_DosFindFirst(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                      ,PLXDOSFINDFIRSTSTRUCT prw)
{
 pParam->rc=DosFindFirst(prw->fileSpec,&prw->hDir,prw->flAttribute
                         ,&prw->LXDOSFINDFIRSTUNION.fileFindBuf3
                         ,prw->cbBuf,&prw->cFileNames
                         ,prw->ulInfoLevel);
}

//-------------------------------- CMD_DosRead ---------------------------------
void CMD_DosRead(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                 ,PLXDOSREADWRITESTRUCT prw)
{
 pParam->rc=DosRead(prw->hFile,prw->data,prw->cbReadWrite,&(prw->cbActual));
}

//-------------------------------- CMD_DosWrite --------------------------------
void CMD_DosWrite(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                  ,PLXDOSREADWRITESTRUCT prw)
{
 pParam->rc=DosWrite(prw->hFile,prw->data,prw->cbReadWrite,&(prw->cbActual));
}

//------------------------------- CMD_DosScanEnv -------------------------------
void CMD_DosScanEnv(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam,char* pData)
{
 PSZ pszVal;
 pParam->rc=DosScanEnv(pData,&pszVal);
 if(!pParam->rc)
  strcpy(pData,pszVal);
}

//---------------------------- CMD_DosQuerySysInfo -----------------------------
void CMD_DosQuerySysInfo(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                         ,PLXDOSQUERYSYSINFOSTRUCT prw)
{
 pParam->rc=DosQuerySysInfo(prw->iStart,prw->iLast,prw,prw->cbBuf);
}

//----------------------------- CMD_DosSetFileSize -----------------------------
void CMD_DosSetFileSize(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                        ,PLXDOSSETFILESIZESTRUCT pfs)
{
 pParam->rc=DosSetFileSize(pfs->hFile,pfs->cbSize);
}

//---------------------------- CMD_DosQueryFileInfo ----------------------------
void CMD_DosQueryFileInfo(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                          ,PLXDOSQUERYFILEINFOSTRUCT pfi)
{
 pParam->rc=DosQueryFileInfo(pfi->hFile,pfi->ulInfoLevel
                             ,&(pfi->filestatus),pfi->cbInfoBuf);
}

//----------------------------- CMD_DosSetFileInfo -----------------------------
void CMD_DosSetFileInfo(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                        ,PLXDOSSETFILEINFOSTRUCT pfi)
{
 pParam->rc=DosSetFileInfo(pfi->hFile,pfi->ulInfoLevel
                           ,&(pfi->filestatus),pfi->cbInfoBuf);
}

//---------------------------- CMD_DosQueryPathInfo ----------------------------
void CMD_DosQueryPathInfo(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                          ,PLXDOSQUERYPATHINFOSTRUCT pi)
{
 pParam->rc=DosQueryPathInfo(pi->pathName,pi->ulInfoLevel
                             ,&pi->LXDOSQUERYPATHINFOUNION.fileStatus3
                             ,pi->cbInfoBuf);
}

//----------------------------- CMD_DosSetPathInfo -----------------------------
void CMD_DosSetPathInfo(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                        ,PLXDOSSETPATHINFOSTRUCT pi)
{
 pParam->rc=DosSetPathInfo(pi->pathName,pi->ulInfoLevel
                           ,&pi->LXDOSSETPATHINFOUNION.fileStatus3
                           ,pi->cbInfoBuf,pi->flOptions);
}


//---------------------------- CMD_DosEnumAttribute ----------------------------
void CMD_DosEnumAttribute(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                          ,PLXDOSENUMATTRIBUTESTRUCT pea)
{
 void* pBuffer=(void*)(((unsigned long)pea)+sizeof(LXDOSENUMATTRIBUTESTRUCT));
 pParam->rc=DosEnumAttribute(pea->ulRefType,&pea->vFile,pea->ulEntry
                             ,pBuffer,pea->cbBuf,&pea->ulCount
                             ,pea->ulInfoLevel);
}

//------------------------------ CMD_EAReadString ------------------------------
void CMD_EAReadString(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                      ,PLXEAREADSTRINGSTRUCT pea)
{
 char *eaVal=(char*)(((ULONG)pea)+sizeof(LXEAREADSTRINGSTRUCT));
 pParam->rc=eahReadStringEA(pea->fileName,pea->eaName,eaVal,&pea->eaValLen);
}

//----------------------------- CMD_EAWriteString ------------------------------
void CMD_EAWriteString(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                       ,PLXEAWRITESTRINGSTRUCT pea)
{
 char *eaVal=(char*)(((ULONG)pea)+sizeof(LXEAWRITESTRINGSTRUCT));
 HFILE hF;
 ULONG action;
 pParam->rc=DosOpen(pea->fileName,&hF,&action,0,FILE_NORMAL
                    ,OPEN_ACTION_FAIL_IF_NEW
                     | OPEN_ACTION_OPEN_IF_EXISTS
                    ,OPEN_FLAGS_WRITE_THROUGH
                     | OPEN_FLAGS_FAIL_ON_ERROR
                     | OPEN_FLAGS_NO_CACHE
                     | OPEN_FLAGS_RANDOMSEQUENTIAL
                     | OPEN_SHARE_DENYNONE
                     | OPEN_ACCESS_READWRITE
                    ,0);
 if(pParam->rc==0 && hF!=0)
 {
  pParam->rc=eahWriteStringEA(hF,pea->eaName,eaVal);
  DosClose(hF);
 }
}

//-------------------------------- CMD_EADelete --------------------------------
void CMD_EADelete(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                  ,PLXTWONAMESTRUCT tn)
{
 pParam->rc=eaPathDeleteOne(tn->name1,tn->name2);
}

//------------------------------- CMD_DosDelete --------------------------------
void CMD_DosDelete(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                   ,PLXONENAMESTRUCT on)
{
 pParam->rc=DosDelete(on->name);
}

//------------------------------ CMD_DosDeleteDir ------------------------------
void CMD_DosDeleteDir(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                      ,PLXONENAMESTRUCT on)
{
 pParam->rc=DosDeleteDir(on->name);
}

//-------------------------------- CMD_DosCopy ---------------------------------
void CMD_DosCopy(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                 ,PLXDOSCOPYSTRUCT cs)
{
 pParam->rc=DosCopy(cs->sourceFile,cs->targetFile,cs->option);
}

//-------------------------------- CMD_DosBeep ---------------------------------
void CMD_DosBeep(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                 ,PLXDOSBEEPSTRUCT p)
{
 pParam->rc=DosBeep(p->frequency,p->duration);
}

//------------------------------ CMD_DosFindNext -------------------------------
void CMD_DosFindNext(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                     ,PLXDOSFINDFIRSTSTRUCT p)
{
 pParam->rc=DosFindNext(p->hDir
                        ,&p->LXDOSFINDFIRSTUNION.fileFindBuf3
                        ,p->cbBuf,&p->cFileNames);
}

//-------------------------------- CMD_DosClose --------------------------------
void CMD_DosClose(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam,PHFILE p)
{
 pParam->rc=DosClose(*p);
}

//----------------------------- CMD_DosSetFilePtr ------------------------------
void CMD_DosSetFilePtr(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam
                       ,PLXDOSSETFILEPTRSTRUCT p)
{
 pParam->rc=DosSetFilePtr(p->hFile,p->ib,p->method
                          ,&p->ibActual);
}

//------------------------------ CMD_DosFindClose ------------------------------
void CMD_DosFindClose(HFILE hFile,LXIOCPA_DMN_CMDPARMPACKET* pParam,PHDIR pData)
{
 pParam->rc=DosFindClose(*pData);
}

