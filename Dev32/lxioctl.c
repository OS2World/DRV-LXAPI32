/* $Id: lxioctl.c,v 1.37 2006/01/05 23:48:15 smilcke Exp $ */

/*
 * lxioctl.c
 * Autor:               Stefan Milcke
 * Erstellt am:         12.11.2001
 * Letzte Aenderung am: 29.12.2005
 *
*/

#include <lxcommon.h>
#include <DEVRP.H>

#include <lxapioctl.h>
#include <lxdaemon.h>
#include "Ver_32.h"

#include <lxapi.h>

#include <linux/string.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <lxapilib.h>

extern unsigned long ioctl_v4lx(struct LX_RP* prp);
extern int total_modules;

char vendor[]="Stefan Milcke";

//--------------------------------- ioctl_deh ----------------------------------
static __inline__ ULONG ioctl_deh(struct LX_RP* prp)
{
 ULONG rc=0;
 switch(prp->RPBODY.RPIOCTL.Function)
 {
  case LXIOCFN_DEH_ALLOCPHYS:
   {
    PARMPACKET(pa,PLXIOCPA_DEH_ALLOCPHYS,prp,VERIFY_READWRITE);
    if(pa)
     pa->rc=DevPhysAlloc(pa->size,(BYTE)pa->high_or_low,(PHYSICAL*)&(pa->pMem));
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_DEH_FREEPHYS:
   {
    PARMPACKET(pa,PLXIOCPA_DEH_FREEPHYS,prp,VERIFY_READWRITE);
    if(pa)
     pa->rc=DevPhysFree((unsigned long)pa->pMem);
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_DEH_VMALLOC:
   {
    PARMPACKET(pa,PLXIOCPA_DEH_VMALLOC,prp,VERIFY_READWRITE);
    if(pa)
    {
     pa->rc=DevVMAlloc(pa->flags,pa->size
                       ,(LINEAR)&(pa->pPhysAddr),(LINEAR*)&(pa->pMem));
    }
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_DEH_VMFREE:
   {
    PARMPACKET(pa,PLXIOCPA_DEH_VMFREE,prp,VERIFY_READWRITE);
    if(pa)
    {
     pa->rc=DevVMFree((LINEAR)pa->pMem);
    }
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_DEH_VMGLOBALTOPROCESS:
   {
    PARMPACKET(pa,PLXIOCPA_DEH_VMGLOBALTOPROCESS,prp,VERIFY_READWRITE);
    if(pa)
    {
     pa->rc=DevVMGlobalToProcess(pa->flags,(LINEAR)pa->pLinearAddr,pa->size,(LINEAR)&(pa->pMem));
    }
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_DEH_VMPROCESSTOGLOBAL:
   {
    PARMPACKET(pa,PLXIOCPA_DEH_VMPROCESSTOGLOBAL,prp,VERIFY_READWRITE);
    if(pa)
    {
     pa->rc=DevVMProcessToGlobal(pa->flags,(LINEAR)pa->pLinearAddr,pa->size,(LINEAR)&(pa->pMem));
    }
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_DEH_VMLOCK:
   {
    PARMPACKET(pa,PLXIOCPA_DEH_VMLOCK,prp,VERIFY_READWRITE);
    if(pa)
     pa->rc=DevVMLock(pa->flags,(unsigned long)pa->pLinearAddr,pa->size
                      ,(LINEAR)&(pa->pPageList)
                      ,(LINEAR)&(pa->lock[0])
                      ,(LINEAR)&(pa->pagelistSize));
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_DEH_VMUNLOCK:
   {
    PARMPACKET(pa,PLXIOCPA_DEH_VMUNLOCK,prp,VERIFY_READWRITE);
    if(pa)
     pa->rc=DevVMUnLock((LINEAR)&(pa->lock[0]));
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_DEH_VDHALLOCPAGES:
   {
    PARMPACKET(pa,PLXIOCPA_DEH_VDHALLOCPAGES,prp,VERIFY_READWRITE);
    /*
    if(pa)
     pa->ObjectAddress=(ULONG)VDHAllocPages(pa->StartingAddress,pa->NumPages,pa->OptionFlag);
    else
     rc=RPERR_PARAMETER;
    */
    if(!pa)
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_DEH_VDHFREEPAGES:
   {
    PARMPACKET(pa,PLXIOCPA_DEH_VDHFREEPAGES,prp,VERIFY_READWRITE);
    /*
    if(pa)
     VDHFreePages((PVOID)pa->ObjectAddress);
    else
     rc=RPERR_PARAMETER;
    */
    if(!pa)
     rc=RPERR_PARAMETER;
   }
   break;
  default:
   rc=RPERR_COMMAND;
 }
 return rc | RPDONE;
}

//-------------------------------- ioctl_global --------------------------------
static __inline__ ULONG ioctl_global(struct LX_RP* prp)
{
 ULONG rc=0;
 switch(prp->RPBODY.RPIOCTL.Function)
 {
  case LXIOCFN_GLO_GETDRIVERINFO:
   {
    DATAPACKET(dp,PLXIOCDP_GLO_GETDRIVERINFO,prp,VERIFY_READWRITE);
    if(dp)
    {
     dp->ulVersionMajor=LX32_DRV_MAJOR_VERSION;
     dp->ulVersionMinor=LX32_DRV_MINOR_VERSION;
     dp->ulBuildLevel=BUILD_LEVEL;
     strcpy(dp->vendor,vendor);
//     dp->ulNumModules=LX_get_total_num_modules();
//     dp->ulV4LXNumDevices=LX_v4lx_get_num_devices();
    }
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_GLO_GETSYSFILENUM:
   {
    DATAPACKET(dp,PLXIOCDP_GLO_GETSYSFILENUM,prp,VERIFY_READWRITE);
    if(dp)
     dp->ulSysFileNum=(ULONG)(prp->RPBODY.RPIOCTL.FileID);
    else
     rc=RPERR_PARAMETER;
   }
   break;
  default:
   rc=RPERR_COMMAND;
   break;
 }
 return rc | RPDONE;
}

//--------------------------------- ioctl_mod ----------------------------------
static __inline__ ULONG ioctl_mod(struct LX_RP* prp)
{
 ULONG rc=0;
 /*
 switch(prp->RPBODY.RPIOCTL.Function)
 {
  case LXIOCFN_MOD_ENUMMODULES:
   {
    DATAPACKET(dp,PLXIOCDP_MOD_ENUMMODULES,prp,VERIFY_READWRITE);
    if(dp)
    {
     PLX_MODINFO pModInfo=dp->modules;
     if(pModInfo)
      dp->ulNumModules=LX_enum_modules(pModInfo,((ULONG)prp->RPIOCTL.DataLength)-sizeof(LXIOCDP_MOD_ENUMMODULES));
    }
   }
   break;
  case LXIOCFN_MOD_REQUEST_MODULE:
   {
    PARMPACKET(p,PLXIOCPA_MOD_REQUEST_MODULE,prp,VERIFY_READWRITE);
    if(p)
     p->rc=request_module(p->name);
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_MOD_RELEASE_MODULE:
   {
    PARMPACKET(p,PLXIOCPA_MOD_RELEASE_MODULE,prp,VERIFY_READWRITE);
    if(p)
     p->rc=release_module(p->name);
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_MOD_SETMODPARM:
   {
    PARMPACKET(p,PLXIOCPA_MOD_SETPARM,prp,VERIFY_READWRITE);
    if(p)
    {
     int sz=prp->RPIOCTL.ParmLength;
     char *mp=(char *)malloc(sz);
     if(mp)
     {
      char errMsg[500];
      memset(mp,0,sz);
      strcpy(mp,p->moduleName);
      mp[strlen(mp)+1]=(char)0;
      mp[strlen(mp)]=':';
      memcpy(&(mp[strlen(mp)]),p->parameter,strlen(p->parameter));
      p->rc=LC_set_module_parm(mp,errMsg);
      free(mp);
     }
     else
     {
      p->rc=-12;
      rc=RPERR;
     }
    }
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_MOD_ENUMMODPARMS:
   {
    PARMPACKET(pa,PLXIOCPA_MOD_ENUMPARMS,prp,VERIFY_READWRITE);
    DATAPACKET(dp,PLXIOCDP_MOD_ENUMPARMS,prp,VERIFY_READWRITE);
    if(dp)
    {
     PLX_MODPARM pModParm=dp->parms;
     pa->rc=LX_enum_module_parms(pa->name,pModParm);
    }
   }
   break;
  default:
   rc=RPERR_COMMAND;
   break;
 }
 */
 return rc | RPDONE;
}

//--------------------------------- ioctl_pci ----------------------------------
static __inline__ ULONG ioctl_pci(struct LX_RP* prp)
{
 ULONG rc=0;
 /*
 switch(prp->RPBODY.RPIOCTL.Function)
 {
  default:
   rc=RPERR_COMMAND;
   break;
 }
 */
 rc=RPERR_COMMAND;
 return rc | RPDONE;
}

//--------------------------------- ioctl_i2c ----------------------------------
static __inline__ ULONG ioctl_i2c(struct LX_RP* prp)
{
 ULONG rc=0;
 /*
 switch(prp->RPBODY.RPIOCTL.Function)
 {
  default:
   rc=RPERR_COMMAND;
   break;
 }
 */
 rc=RPERR_COMMAND;
 return rc | RPDONE;
}
extern void* vmalloc(unsigned long size);
extern void vfree(void *ptr);
extern void* LX_map_vm_to_user(void* ptr,unsigned long size);
extern void* LX_map_to_phys(void* ptr,unsigned long size);
extern void* LX_unmap_to_phys(void* virtAddr,void* physAddr);

//--------------------------------- ioctl_mem ----------------------------------
static __inline__ ULONG ioctl_mem(struct LX_RP* prp)
{
 ULONG rc=0;
 switch(prp->RPBODY.RPIOCTL.Function)
 {
  case LXIOCFN_MEM_VMALLOC:
   {
    PARMPACKET(pa,PLXIOCPA_MEM_VMALLOC,prp,VERIFY_READWRITE);
    DATAPACKET(dp,PLXIOCDP_MEM_VMALLOC,prp,VERIFY_READWRITE);
    pa->rc=-EINVAL;
    if(pa && dp)
    {
     dp->kernelMem=vmalloc(pa->size);
     if(dp->kernelMem)
     {
      dp->physMem=(void*)virt_to_phys(dp->kernelMem);
      if(dp->physMem)
      {
       dp->virtMem=(void*)LX_map_vm_to_user(dp->kernelMem,pa->size);
       pa->rc=0;
      }
     }
    }
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_MEM_VFREE:
   {
    PARMPACKET(pa,PLXIOCPA_MEM_VFREE,prp,VERIFY_READWRITE);
    vfree(pa->kernelMem);
    pa->rc=0;
   }
   break;
/*
  case LXIOCFN_MEM_MAPTOPHYS:
   {
    PARMPACKET(pa,PLXIOCPA_MEM_MAPTOPHYS,prp,VERIFY_READWRITE);
    DATAPACKET(dp,PLXIOCDP_MEM_MAPTOPHYS,prp,VERIFY_READWRITE);
    pa->rc=-EINVAL;
    if(pa && dp)
    {
     dp->physMem=LX_map_to_phys(pa->virtMem,pa->size);
     if(dp->physMem)
      pa->rc=0;
    }
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_MEM_UNMAPTOPHYS:
   {
    PARMPACKET(pa,PLXIOCPA_MEM_UNMAPTOPHYS,prp,VERIFY_READWRITE);
    if(pa)
    {
     LX_unmap_to_phys(pa->virtMem,pa->physMem);
     pa->rc=0;
    }
    else
     rc=RPERR_PARAMETER;
   }
   break;
*/
  case LXIOCFN_MEM_ALLOCPHYS:
   {
    PARMPACKET(pa,PLXIOCPA_MEM_ALLOCPHYS,prp,VERIFY_READWRITE);
    DATAPACKET(dp,PLXIOCDP_MEM_ALLOCPHYS,prp,VERIFY_READWRITE);
    if(pa && dp)
    {
     pa->rc=DevPhysAlloc(pa->size,0,(unsigned long*)&(dp->physMem));
     if(!dp->physMem)
      pa->rc=DevPhysAlloc(pa->size,1,(unsigned long*)&(dp->physMem));
     if(dp->physMem)
      pa->rc=0;
    }
    else
     rc=RPERR_PARAMETER;
   }
   break;
  case LXIOCFN_MEM_FREEPHYS:
   {
    PARMPACKET(pa,PLXIOCPA_MEM_FREEPHYS,prp,VERIFY_READWRITE);
    if(pa)
     pa->rc=DevPhysFree((unsigned long)pa->physMem);
    else
     rc=RPERR_PARAMETER;
   }
   break;
  default:
   rc=RPERR_COMMAND;
   break;
 }
 return rc | RPDONE;
}

//-------------------------------- map_lx_error --------------------------------
static  __inline__ ULONG map_lx_error(int err)
{
 return RPDEV | (-err);
}

//------------------------------ V4L_EnumDevices -------------------------------
static __inline__ ULONG V4L_EnumDevices(struct LX_RP* prp)
{
 ULONG rc=RPDONE;
 /*
 PLXIOCDP_V4L_ENUMDEVICES pEnum=(PLXIOCDP_V4L_ENUMDEVICES)
          VDPFROMRP(prp,prp->RPIOCTL.DataLength,VERIFY_READWRITE);
 if(pEnum)
 {
  PLX_V4L_DEVICEINFO p=(PLX_V4L_DEVICEINFO)pEnum->devInfos;
  if(p)
   pEnum->ulDevices=LX_v4lx_enum_devices(p,((ULONG)prp->RPIOCTL.DataLength)-sizeof(LXIOCDP_V4L_ENUMDEVICES));
 }
 else
  rc|=RPERR_PARAMETER;
 */
 return rc;
}

//------------------------------- V4L_OpenDevice -------------------------------
static __inline__ ULONG V4L_OpenDevice(struct LX_RP* prp)
{
 ULONG rc=RPDONE;
 /*
 PLXIOCPA_V4L_OPENDEVICE paOpen=(PLXIOCPA_V4L_OPENDEVICE)
           VPAFROMRP(prp,prp->RPIOCTL.ParmLength,VERIFY_READWRITE);
 paOpen->rc=LX_v4lx_open_device(paOpen->devname,prp->RPIOCTL.FileID,paOpen->autofree_flags);
 if(paOpen->rc>0)
 {
  paOpen->handle=paOpen->rc;
  paOpen->rc=0;
 }
 else
  rc|=map_lx_error(paOpen->rc);
 */
 return rc;
}

//------------------------------ V4L_CloseDevice -------------------------------
static __inline__ ULONG V4L_CloseDevice(struct LX_RP* prp)
{
 ULONG rc=RPDONE;
 /*
 PLXIOCPA_V4L_CLOSEDEVICE paClose=(PLXIOCPA_V4L_CLOSEDEVICE)
           VPAFROMRP(prp,prp->RPIOCTL.ParmLength,VERIFY_READWRITE);
 paClose->rc=LX_v4lx_close_device(paClose->handle);
 rc|=map_lx_error(paClose->rc);
 */
 return rc;
}

//--------------------------------- V4L_IOCtl ----------------------------------
static __inline__ ULONG V4L_IOCtl(struct LX_RP* prp)
{
 ULONG rc=RPDONE;
 /*
 PLXIOCPA_V4L_IOCTL paIoctl=(PLXIOCPA_V4L_IOCTL)
           VPAFROMRP(prp,prp->RPIOCTL.ParmLength,VERIFY_READWRITE);
 PLXIOCDP_V4L_IOCTL dpIoctl=(PLXIOCDP_V4L_IOCTL)
           VDPFROMRP(prp,prp->RPIOCTL.DataLength,VERIFY_READWRITE);
 paIoctl->rc=LX_v4lx_ioctl(paIoctl->handle
                            ,paIoctl->v4l_ioctlfn
                            ,dpIoctl);
 */
 return rc;
}

//---------------------------------- V4L_MMap ----------------------------------
static __inline__ ULONG V4L_MMap(struct LX_RP* prp)
{
 ULONG rc=RPDONE;
 /*
 void *mmap=NULL;
 PLXIOCPA_V4L_MMAP paMmap=(PLXIOCPA_V4L_MMAP)
           VPAFROMRP(prp,prp->RPIOCTL.ParmLength,VERIFY_READWRITE);
 PLXIOCDP_V4L_MMAP dpMmap=(PLXIOCDP_V4L_MMAP)
           VDPFROMRP(prp,prp->RPIOCTL.DataLength,VERIFY_READWRITE);
 paMmap->rc=LX_v4lx_mmap(paMmap->handle,&mmap,paMmap->size);
 dpMmap->mmap=mmap;
 */
 return rc;
}

//---------------------------------- V4L_Read ----------------------------------
static __inline__ ULONG V4L_Read(struct LX_RP* prp)
{
 ULONG rc=RPDONE;
 /*
 PARMPACKET(pa,PLXIOCPA_V4L_READ,prp,VERIFY_READWRITE);
 DATAPACKET(dp,PLXIOCDP_V4L_READ,prp,VERIFY_READWRITE);
 if(pa && dp)
 {
  ULONG numLocks=1;
  char lock[12*5];
  if(!LX_lock_mem((PVOID)dp->buffer,pa->count
                   ,VMDHL_WRITE | VMDHL_LONG
                   ,(void*)lock
                   ,&numLocks))
  {
   pa->rc=LX_v4lx_read(pa->handle,dp->buffer,pa->count,pa->nonblock);
   LX_unlock_mem((void*)lock,numLocks);
  }
 }
 else
  rc|=RPERR_PARAMETER;
 */
 return rc;
}

extern ULONG LX_ioctl_dmn(struct LX_RP* prp);

//------------------------------- LX_StratIOCtl --------------------------------
ULONG LX_StratIOCtl(struct LX_RP* prp)
{
 ULONG rc=RPERR_COMMAND | RPDONE;
 HLOCK hLock=0;
 current_fileId=prp->RPBODY.RPIOCTL.FileID;
 LX_set_current_dev_fileId(prp->RPBODY.RPIOCTL.FileID);
 if(prp->RPBODY.RPIOCTL.DataLength)
  DevSegLock(SELECTOROF(prp->RPBODY.RPIOCTL.DataPacket),1,0,&hLock);
 switch(prp->RPBODY.RPIOCTL.Category)
 {
  case LXIOCCAT_GLO:
   rc=ioctl_global(prp);
   break;
  case LXIOCCAT_MOD:
   rc=ioctl_mod(prp);
   break;
  case LXIOCCAT_PCI:
   rc=ioctl_pci(prp);
   break;
  case LXIOCCAT_I2C:
   rc=ioctl_i2c(prp);
   break;
  case LXIOCCAT_MEM:
   rc=ioctl_mem(prp);
   break;
  case LXIOCCAT_V4L:
   rc=RPDONE;
   switch(prp->RPBODY.RPIOCTL.Function)
   {
    case LXIOCFN_V4L_ENUMDEVICES:
     rc=V4L_EnumDevices(prp);
     break;
    case LXIOCFN_V4L_OPENDEVICE:
     rc=V4L_OpenDevice(prp);
     break;
    case LXIOCFN_V4L_CLOSEDEVICE:
     rc=V4L_CloseDevice(prp);
     break;
    case LXIOCFN_V4L_IOCTL:
     rc=V4L_IOCtl(prp);
     break;
    case LXIOCFN_V4L_MMAP:
     rc=V4L_MMap(prp);
     break;
    case LXIOCFN_V4L_READ:
     rc=V4L_Read(prp);
     break;
    default:
     rc|=RPERR_COMMAND;
     break;
   }
   break;
  case LXIOCCAT_DEH:
   rc=ioctl_deh(prp);
   break;
  case LXIOCCAT_DMN:
   rc=LX_ioctl_dmn(prp);
   break;
 }
 LX_set_current_dev_fileId(0);
 if(hLock && prp->RPBODY.RPIOCTL.DataLength)
  DevSegUnlock(hLock);
 return rc;
}
