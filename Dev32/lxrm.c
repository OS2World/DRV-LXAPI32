/* $Id: lxrm.c,v 1.17 2006/01/05 23:48:16 smilcke Exp $ */

/*
 * lxrm.c
 * Autor:               Stefan Milcke
 * Erstellt am:         20.02.2002
 * Letzte Aenderung am: 29.12.2005
 *
*/

#include <lxcommon.h>
#include <devrp.h>

#include <lxrmcall.h>

#include "ver_32.h"

#include <asm/bitops.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
#include <asm/hardirq.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/i2c.h>

static HDRIVER hDriver=0;
static HADAPTER hPCIAdap=0;
static HDEVICE hPCIDev=0;
static HADAPTER hMEMAdap=0;
static char LXDRIVER_NAME[]="LXAPI32.SYS";
static char LXVENDOR_NAME[]="Stefan Milcke";

extern unsigned long lx_BuildDay;
extern unsigned long lx_BuildMonth;
extern unsigned long lx_BuildYear;

int dolxdevtree=0;
int dorm=1;

/*******************************************************************************/
/* RM-Routers                                                                 */
/*******************************************************************************/
HDRIVER *rm32_hDriverPtr=NULL;
HADAPTER* rm32_hAdapterPtr=NULL;
HDEVICE* rm32_hDevicePtr=NULL;
HRESOURCE* rm32_hResourcePtr=NULL;
ULONG* rm32_ModifyActionPtr=NULL;
ADAPTERSTRUCT* rm32_AdapterStructPtr=NULL;
DRIVERSTRUCT* rm32_DriverStructPtr=NULL;
DEVICESTRUCT* rm32_DeviceStructPtr=NULL;
RESOURCESTRUCT* rm32_ResourceStructPtr=NULL;
char* rm32_ResourceListPtr=NULL;
char* rm32_AdjunctBufferPtr=NULL;
char* rm32_tmpStr1Ptr=NULL;
char* rm32_tmpStr2Ptr=NULL;
char* rm32_tmpStr3Ptr=NULL;

//-------------------------------- RMCopyToStr ---------------------------------
static unsigned long RMCopyToStr(char *dest,char* src)
{
 unsigned long i=0;
 while(1)
 {
  dest[i]=src[i];
  if(!src[i])
   break;
  i++;
 }
 return i;
}
//-------------------------------- RMCopyToStr1 --------------------------------
static __inline__ unsigned long RMCopyToStr1(char* src)
{
 return RMCopyToStr(rm32_tmpStr1Ptr,src);
}

//-------------------------------- RMCopyToStr2 --------------------------------
static __inline__ unsigned long RMCopyToStr2(char* src)
{
 return RMCopyToStr(rm32_tmpStr2Ptr,src);
}

//-------------------------------- RMCopyToStr3 --------------------------------
static __inline__ unsigned long RMCopyToStr3(char* src)
{
 return RMCopyToStr(rm32_tmpStr3Ptr,src);
}

//----------------------------- RMCopyAdjunctList ------------------------------
static void RMCopyAdjunctList(ADJUNCT *pAdjunct)
{
 register int i,c=0;
 char *p=(char*)rm32_AdjunctBufferPtr;
 if(pAdjunct)
 {
  while(pAdjunct)
  {
   for(i=0;i<pAdjunct->AdjLength;i++)
    p[i+c]=((char*)pAdjunct)[i];
   c+=i;
   pAdjunct=pAdjunct->pNextAdj;
  }
 }
 else
 {
  for(i=0;i<4;i++)
   p[i]=(char)0;
 }
}

//----------------------------- RMCopyResourceList -----------------------------
static void RMCopyResourceList(AHRESOURCE* pAHResource)
{
 register int i;
 char* p=(char *)rm32_ResourceListPtr;
 if(pAHResource)
 {
  for(i=0;i<4;i++)
   p[i]=((char*)pAHResource)[i];
 }
 else
 {
  for(i=0;i<4;i++)
   p[i]=(char)0;
 }
}

__attribute__((regparm(3)))
extern RM32RET RM32CreateDriverASM(void);
//#pragma aux RM32CreateDriverASM value [eax] modify [eax ebx ecx edx esi edi]

//------------------------------ RM32CreateDriver ------------------------------
RM32RET __cdecl RM32CreateDriver(DRIVERSTRUCT* pDrvStruct,HDRIVER* pHDriver)
{
 RM32RET rc=0;
 register int i;
 char* p;
 if(dorm)
 {
  // Copy DrvrName,DrvrDexcript,VendorName to temporary strings
  RMCopyToStr1((char*)pDrvStruct->DrvrName);
  RMCopyToStr2((char*)pDrvStruct->DrvrDescript);
  RMCopyToStr3((char*)pDrvStruct->VendorName);
  // Copy driver struct
  p=(char *)(rm32_DriverStructPtr);
  for(i=12;i<12+16;i++)
   p[i]=((char*)pDrvStruct)[i];
  // Call RM (Thunking)
  rc=RM32CreateDriverASM();
  if(!rc)
   *pHDriver=*((HDRIVER*)(rm32_hDriverPtr));
 }
 return rc;
}

__attribute__((regparm(3)))
RM32RET RM32DestroyDriverASM(void);
//#pragma aux RM32DestroyDriverASM value [eax] modify [eax ebx ecx edx esi edi]

//----------------------------- RM32DestroyDriver ------------------------------
RM32RET __cdecl RM32DestroyDriver(HDRIVER hDriver)
{
 if(dorm)
 {
  HDRIVER* pHSrc=(HDRIVER*)rm32_hDriverPtr;
  *pHSrc=hDriver;
  return RM32DestroyDriverASM();
 }
 else
  return 0;
}

__attribute__((regparm(3)))
RM32RET RM32CreateAdapterASM(void);
//#pragma aux RM32CreateAdapterASM value [eax] modify [eax ebx ecx edx esi edi]

//----------------------------- RM32CreateAdapter ------------------------------
RM32RET __cdecl RM32CreateAdapter(HDRIVER hDrv,HADAPTER* pHAdap
                                  ,ADAPTERSTRUCT* pAdapStruct
                                  ,HDEVICE hDev
                                  ,AHRESOURCE* pAHResource)
{
 RM32RET rc=0;
 register int i;
 char* p;
 if(dorm)
 {
  // Copy AdaptDescriptName
  RMCopyToStr1((char*)pAdapStruct->AdaptDescriptName);
  // Copy hDrv and hDev
  {
   HDRIVER* pHDrv=(HDRIVER*)rm32_hDriverPtr;
   HDEVICE* pHDev=(HDEVICE*)rm32_hDevicePtr;
   *pHDrv=hDrv;
   *pHDev=hDev;
  }
  // Copy Adapter struct
  p=(char*)rm32_AdapterStructPtr;
  for(i=4;i<4+20;i++)
   p[i]=((char*)pAdapStruct)[i];
  // Copy ResourceList
  RMCopyResourceList(pAHResource);
  // Copy Adjunct list
  RMCopyAdjunctList(pAdapStruct->pAdjunctList);
  rc=RM32CreateAdapterASM();
  if(!rc)
  {
   HADAPTER* pHAd=(HADAPTER*)rm32_hAdapterPtr;
   *pHAdap=*pHAd;
  }
 }
 return rc;
}

__attribute__((regparm(3)))
RM32RET RM32DestroyAdapterASM(void);
//#pragma aux RM32DestroyAdapterASM value [eax] modify [eax ebx ecx edx esi edi]

//----------------------------- RM32DestroyAdapter -----------------------------
RM32RET __cdecl RM32DestroyAdapter(HDRIVER hDrv,HADAPTER hAdap)
{
 if(dorm)
 {
  HDRIVER* pHDrv=(HDRIVER*)rm32_hDriverPtr;
  HADAPTER* pHAd=(HADAPTER*)rm32_hAdapterPtr;
  *pHDrv=hDrv;
  *pHAd=hAdap;
  return RM32DestroyAdapterASM();
 }
 else
  return 0;
}

__attribute__((regparm(3)))
RM32RET RM32CreateDeviceASM(void);
//#pragma aux RM32CreateDeviceASM value [eax] modify [eax ebx ecx edx esi edi]

//------------------------------ RM32CreateDevice ------------------------------
RM32RET __cdecl RM32CreateDevice(HDRIVER hDrv,HDEVICE* pHDev
                                 ,DEVICESTRUCT* pDevStruct
                                 ,HADAPTER hAdap
                                 ,AHRESOURCE* pAHResource)
{
 RM32RET rc=0;
 register int i;
 char* p;
 if(dorm)
 {
  // Copy DevDescriptName
  RMCopyToStr1((char*)pDevStruct->DevDescriptName);
  // Copy hDrv and hAdap
  {
   HDRIVER* pHDrv=(HDRIVER*)rm32_hDriverPtr;
   HADAPTER* pHAd=(HADAPTER*)rm32_hAdapterPtr;
   *pHDrv=hDrv;
   *pHAd=hAdap;
  }
  // Copy Device struct
  p=(char *)rm32_DeviceStructPtr;
  for(i=4;i<4+8;i++)
   p[i]=((char*)pDevStruct)[i];
  // Copy ResourceList
  RMCopyResourceList(pAHResource);
  // Copy Adjunct list
  RMCopyAdjunctList(pDevStruct->pAdjunctList);
  rc=RM32CreateDeviceASM();
  if(!rc)
  {
   HDEVICE* pHDe=(HDEVICE*)rm32_hDevicePtr;
   *pHDev=*pHDe;
  }
 }
 return rc;
}

__attribute__((regparm(3)))
RM32RET RM32DestroyDeviceASM(void);
//#pragma aux RM32DestroyDeviceASM value [eax] modify [eax ebx ecx edx esi edi]

//----------------------------- RM32DestroyDevice ------------------------------
RM32RET __cdecl RM32DestroyDevice(HDRIVER hDrv,HDEVICE hDev)
{
 if(dorm)
 {
  HDRIVER* pHDrv=(HDRIVER*)rm32_hDriverPtr;
  HDEVICE* pHDe=(HDEVICE*)rm32_hDevicePtr;
  *pHDrv=hDrv;
  *pHDe=hDev;
  return RM32DestroyDeviceASM();
 }
 else
  return 0;
}

__attribute__((regparm(3)))
RM32RET RM32AllocResourceASM(void);
//#pragma aux RM32AllocResourceASM value [eax] modify [eax ebx ecx edx esi edi]

//----------------------------- RM32AllocResource ------------------------------
RM32RET __cdecl RM32AllocResource(HDRIVER hDrv,HRESOURCE* pHResource
                                  ,RESOURCESTRUCT* pResourceStruct)
{
 RM32RET rc=0;
 register int i;
 char* p;
 if(dorm)
 {
  // Copy hDrv
  {
   HDRIVER* pHDrv=(HDRIVER*)rm32_hDriverPtr;
   *pHDrv=hDrv;
  }
  // Copy Resource Struct
  p=(char*)rm32_ResourceStructPtr;
  for(i=0;i<20;i++)
   p[i]=((char*)pResourceStruct)[i];
  rc=RM32AllocResourceASM();
  if(!rc)
  {
   HRESOURCE* pHRe=(HRESOURCE*)rm32_hResourcePtr;
   *pHResource=*pHRe;
  }
 }
 return rc;
}

__attribute__((regparm(3)))
RM32RET RM32DeallocResourceASM(void);
//#pragma aux RM32DeallocResourceASM value [eax] modify [eax ebx ecx edx esi edi]

//---------------------------- RM32DeallocResource -----------------------------
RM32RET __cdecl RM32DeallocResource(HDRIVER hDrv,HRESOURCE hRes)
{
 if(dorm)
 {
  // Copy hDrv and hRes
  {
   HDRIVER* pHDrv=(HDRIVER*)rm32_hDriverPtr;
   HRESOURCE* pHRe=(HRESOURCE*)rm32_hResourcePtr;
   *pHDrv=hDrv;
   *pHRe=hRes;
  }
  return RM32DeallocResourceASM();
 }
 else
  return 0;
}

__attribute__((regparm(3)))
RM32RET RM32ModifyResourcesASM(void);
//#pragma aux RM32ModifyResourcesASM value [eax] modify [eax ebx ecx edx esi edi]

//---------------------------- RM32ModifyResources -----------------------------
RM32RET __cdecl RM32ModifyResources(HDRIVER hDrv,HADAPTER hAdap
                                    ,USHORT ModifyAction,HRESOURCE hRes)
{
 if(dorm)
 {
  // Copy hDrv, hAdap and hRes
  {
   HDRIVER* pHDrv=(HDRIVER*)rm32_hDriverPtr;
   HADAPTER* pHAd=(HADAPTER*)rm32_hAdapterPtr;
   HRESOURCE* pHRe=(HRESOURCE*)rm32_hResourcePtr;
   *pHDrv=hDrv;
   *pHAd=hAdap;
   *pHRe=hRes;
  }
  // Copy MofifyAction
  {
   USHORT* pMo=(USHORT*)rm32_ModifyActionPtr;
   *pMo=ModifyAction;
  }
  return RM32ModifyResourcesASM();
 }
 else
  return 0;
}

/*******************************************************************************/
/* OS2 helper functions                                                       */
/*******************************************************************************/
//--------------------------- LX_init_driver_struct ----------------------------
void LX_init_driver_struct(PDRIVERSTRUCT pDriver)
{
 pDriver->DrvrName=(PSZ)LXDRIVER_NAME;
 pDriver->DrvrDescript=(PSZ)"Linux driver";
 pDriver->VendorName=(PSZ)LXVENDOR_NAME;
 pDriver->MajorVer=LX32_DRV_MAJOR_VERSION;
 pDriver->MinorVer=LX32_DRV_MINOR_VERSION;
 pDriver->Date.Year=lx_BuildYear;
 pDriver->Date.Month=lx_BuildMonth;
 pDriver->Date.Day=lx_BuildDay;
 pDriver->DrvrFlags=0;
 pDriver->DrvrType=0;
 pDriver->DrvrSubType=0;
 pDriver->DrvrCallback=NULL;
}

//--------------------------- LX_create_pci_adapter ----------------------------
unsigned long LX_create_pci_adapter(void)
{
 unsigned long rc=0;
 if(!dolxdevtree || !dorm)
  return rc;
 if(!hPCIAdap)
 {
  ADAPTERSTRUCT AdapterStruct;
  ADJUNCT AdapNum;
  AdapterStruct.AdaptDescriptName=(PSZ)"Linux PCI bus";
  AdapterStruct.AdaptFlags=AS_NO16MB_ADDRESS_LIMIT;
  AdapterStruct.BaseType=AS_BASE_RESERVED;
  AdapterStruct.SubType=AS_SUB_OTHER;
  AdapterStruct.InterfaceType=AS_INTF_GENERIC;
  AdapterStruct.HostBusType=AS_HOSTBUS_PCI;
  AdapterStruct.HostBusWidth=AS_BUSWIDTH_32BIT;
  AdapterStruct.pAdjunctList=&AdapNum;
  AdapNum.pNextAdj=NULL;
  AdapNum.AdjLength=sizeof(ADJUNCT);
  AdapNum.AdjType=ADJ_ADAPTER_NUMBER;
  AdapNum.ADJUNCTBODY.Adapter_Number=0;
  rc=RM32CreateAdapter(hDriver,&hPCIAdap,&AdapterStruct,0,NULL);
 }
 if(!rc && !hPCIDev)
 {
  DEVICESTRUCT DevStruct;
  DevStruct.DevDescriptName=(PSZ)"Linux PCI device tree";
  DevStruct.DevFlags=DS_FIXED_LOGICALNAME;
  DevStruct.DevType=DS_TYPE_UNKNOWN;
  DevStruct.pAdjunctList=NULL;
  rc=RM32CreateDevice(hDriver,&hPCIDev,&DevStruct,hPCIAdap,NULL);
 }
 return rc;
}

//---------------------------- LX_pci_convert_class ----------------------------
static __inline__ void LX_pci_convert_class(struct pci_dev *dev,PULONG pCl,PULONG pScl)
{
 ULONG cv=128;
 ULONG cl=(dev->class>>16)&0x0f;
 ULONG scl=(dev->class>>8)&0x0f;
// ULONG pi=(dev->class)&0x0f;
 if(cl<=0x0d)
 {
  if(scl<128)
  {
   scl++;
   switch(cl)
   {
    case AS_BASE_MSD:
    case AS_BASE_PERIPH:
     cv=4;
     break;
    case AS_BASE_NETWORK:
    case AS_BASE_COMM:
    case AS_BASE_INPUT:
     cv=3;
     break;
    case AS_BASE_DISPLAY:
    case AS_BASE_MMEDIA:
     cv=2;
     break;
    case AS_BASE_MEMORY:
    case AS_BASE_PCMCIA:
     cv=1;
     break;
    case AS_BASE_BRIDGE:
    case AS_BASE_DOCK:
    case AS_BASE_CPU:
    case AS_BASE_BIOS_ROM:
     cv=0;
     break;
    default:
     cv=0;
     break;
   }
   if(scl>cv)
    scl=AS_SUB_OTHER;
  }
 }
 else
 {
  cl=AS_BASE_RESERVED;
  scl=AS_SUB_OTHER;
 }
 *pCl=cl;
 *pScl=scl;
}

//----------------------------- LX_pci_name_device -----------------------------
unsigned long LX_pci_name_device(struct pci_dev *dev,char *vname,char *dname,char *sname)
{
 unsigned long rc=0;
/*
 struct lxrm_subadapter *rm_adap=dev->rm_subadapter;
 ULONG cl=AS_BASE_RESERVED,scl=AS_SUB_OTHER;
 if(!dolxdevtree || !dorm)
  return rc;
 if(rm_adap && !rm_adap->hAdapter)
 {
  ADAPTERSTRUCT AdapStruct;
  PADAPTERSTRUCT pAdap=rm_adap->pAdapter;
  static int devno=0;
  ADJUNCT AdapNumber;
  ADJUNCT AdapDeviceNr;
  ADJUNCT AdapPCI_DevFunc;
  AdapNumber.pNextAdj=&AdapDeviceNr;
  AdapDeviceNr.pNextAdj=&AdapPCI_DevFunc;
  AdapPCI_DevFunc.pNextAdj=NULL;
  AdapNumber.AdjLength=sizeof(ADJUNCT);
  AdapDeviceNr.AdjLength=sizeof(ADJUNCT);
  AdapPCI_DevFunc.AdjLength=sizeof(ADJUNCT);
  AdapNumber.AdjType=ADJ_ADAPTER_NUMBER;
  AdapDeviceNr.AdjType=ADJ_DEVICE_NUMBER;
  AdapPCI_DevFunc.AdjType=ADJ_PCI_DEVFUNC;
  if(dev->bus)
   AdapNumber.Adapter_Number=dev->bus->number;
  else
   AdapNumber.Adapter_Number=0;
  AdapDeviceNr.Device_Number=devno;
  AdapPCI_DevFunc.PCI_DevFunc=(USHORT)dev->devfn;
  if(NULL==rm_adap->hDriver)
   rm_adap->hDriver=hDriver;
  if(NULL==rm_adap->hParentDevice)
  {
   rc=LX_create_pci_adapter();
   if(rc)
    return rc;
   rm_adap->hParentDevice=hPCIDev;
  }
  if(NULL==pAdap)
  {
   pAdap=&AdapStruct;
   LX_pci_convert_class(dev,&cl,&scl);
   AdapStruct.AdaptDescriptName=(PSZ)dev->name;
   AdapStruct.AdaptFlags=AS_NO16MB_ADDRESS_LIMIT;
   AdapStruct.BaseType=(USHORT)cl;
   AdapStruct.SubType=(USHORT)scl;
   AdapStruct.InterfaceType=AS_INTF_GENERIC;
   AdapStruct.HostBusType=AS_HOSTBUS_PCI;
   AdapStruct.HostBusWidth=AS_BUSWIDTH_32BIT;
   AdapStruct.pAdjunctList=&AdapNumber;
   AdapDeviceNr.Device_Number=devno++;
   AdapPCI_DevFunc.PCI_DevFunc=(unsigned short)dev->devfn;
  }
  rc=RM32CreateAdapter(rm_adap->hDriver
                       ,&(rm_adap->hAdapter)
                       ,pAdap
                       ,rm_adap->hParentDevice
                       ,NULL);
  if(0==rc && 0!=dev->subsystem_vendor)
  {
   struct lxrm_subdevice *rm_dev=dev->rm_subdevice;
   if(rm_dev && !rm_dev->hDevice)
   {
    DEVICESTRUCT DevStruct;
    PDEVICESTRUCT pDev=rm_dev->pDevice;
    if(NULL==rm_dev->hParentAdapter)
     rm_dev->hParentAdapter=rm_adap->hAdapter;
    if(NULL==pDev)
    {
     pDev=&DevStruct;
     DevStruct.DevDescriptName=(PSZ)sname;
     DevStruct.DevFlags=DS_FIXED_LOGICALNAME;
     DevStruct.DevType=DS_TYPE_UNKNOWN;
     DevStruct.pAdjunctList=NULL;
    }
    if(NULL==rm_dev->hDriver)
     rm_dev->hDriver=hDriver;
    rc=RM32CreateDevice(rm_dev->hDriver
                        ,&(dev->rm_subdevice->hDevice)
                        ,pDev
                        ,rm_dev->hParentAdapter
                        ,NULL);
   }
  }
 }
*/
 return rc;
}

//--------------------------- LX_pci_register_driver ---------------------------
unsigned long LX_pci_register_driver(struct pci_driver *drv)
{
 unsigned long rc=0;
 /*
 if(!dolxdevtree || !dorm)
  return rc;
 if((NULL!=drv->rm_driver) && (0==drv->rm_driver->hDriver))
 {
  DRIVERSTRUCT DrvStruct;
  PDRIVERSTRUCT pDrv=drv->rm_driver->pDriver;
  if(NULL==pDrv)
  {
   pDrv=&DrvStruct;
   LX_init_driver_struct(pDrv);
   DrvStruct.DrvrDescript=(PSZ)"Linux PCI support layer";
   DrvStruct.DrvrType=DRT_SERVICE;
  }
  rc=RM32CreateDriver(pDrv,&(drv->rm_driver->hDriver));
 }
 */
 return rc;
}

//-------------------------- LX_pci_unregister_driver --------------------------
unsigned long LX_pci_unregister_driver(struct pci_driver *drv)
{
 unsigned long rc=0;
 /*
 if(!dolxdevtree || !dorm)
  return rc;
 if((NULL!=drv->rm_driver) && (0!=drv->rm_driver->hDriver))
 {
  rc=RM32DestroyDriver(drv->rm_driver->hDriver);
  drv->rm_driver->hDriver=0;
 }
 */
 return rc;
}

//--------------------------- LX_pci_announce_device ---------------------------
unsigned long LX_pci_announce_device(struct pci_driver *drv,struct pci_dev *dev)
{
 unsigned long rc=0;
 /*
 if(!dolxdevtree || !dorm)
  return rc;
 if((NULL!=dev->rm_device) && (0==dev->rm_device->hDevice))
 {
  DEVICESTRUCT DevStruct;
  PDEVICESTRUCT pDev=dev->rm_device->pDevice;
  HDRIVER hDrvr=dev->rm_device->hDriver;
  HADAPTER hAdap=dev->rm_device->hAdapter;
  if(NULL==hDrvr)
   hDrvr=hDriver;
  if(NULL==pDev)
  {
   pDev=&DevStruct;
   DevStruct.DevDescriptName=(PSZ)dev->name;
   DevStruct.DevFlags=DS_FIXED_LOGICALNAME;
   DevStruct.DevType=DS_TYPE_UNKNOWN;
   DevStruct.pAdjunctList=NULL;
  }
  rc=RM32CreateDevice(hDrvr,&(dev->rm_device->hDevice),pDev,hAdap,NULL);
 }
 */
 return rc;
}

//----------------------------- LX_i2c_add_driver ------------------------------
unsigned long LX_i2c_add_driver(struct i2c_driver *driver)
{
 unsigned long rc=0;
 /*
 if(!dolxdevtree || !dorm)
  return rc;
 if((NULL!=driver->rm_device) && (0==driver->rm_device->hDevice))
 {
  DEVICESTRUCT DevStruct;
  PDEVICESTRUCT pDev=driver->rm_device->pDevice;
  HDRIVER hDrvr=driver->rm_device->hDriver;
  HADAPTER hAdap=driver->rm_device->hAdapter;
  if(NULL==hDrvr)
   hDrvr=hDriver;
  if(NULL==pDev)
  {
   pDev=&DevStruct;
   DevStruct.DevDescriptName=(PSZ)driver->name;
   DevStruct.DevFlags=DS_FIXED_LOGICALNAME;
   DevStruct.DevType=DS_TYPE_UNKNOWN;
   DevStruct.pAdjunctList=NULL;
  }
  rc=RM32CreateDevice(hDrvr,&(driver->rm_device->hDevice),pDev,hAdap,NULL);
 }
 */
 return rc;
}

//----------------------------- LX_i2c_del_driver ------------------------------
unsigned long LX_i2c_del_driver(struct i2c_driver *driver)
{
 unsigned long rc=0;
 /*
 if(!dolxdevtree || !dorm)
  return rc;
 if((NULL!=driver->rm_device) && (0==driver->rm_device->hDevice))
 {
  HDRIVER hDrvr=driver->rm_device->hDriver;
  HADAPTER hAdap=driver->rm_device->hAdapter;
  if(NULL==hDrvr)
   hDrvr=hDriver;
  rc=RM32DestroyDevice(hDrvr,driver->rm_device->hDevice);
  driver->rm_device->hDevice=0;
 }
 */
 return rc;
}

//------------------------------- LX_request_irq -------------------------------
unsigned long LX_request_irq(struct lxrm_resource *rm_resource
                              ,unsigned int irq,unsigned long flags)
{
 unsigned long rc=0;
 /*
 if(!dolxdevtree || !dorm)
  return rc;
 if(rm_resource && (0==rm_resource->hResource))
 {
  RESOURCESTRUCT IRQResStruct;
  PRESOURCESTRUCT pIRQResStruct=rm_resource->pResource;
  if(0==rm_resource->hDriver)
   rm_resource->hDriver=hDriver;
  if(0==rm_resource->hAdapterOrDevice)
   rm_resource->hAdapterOrDevice=hIRQAdap;
  if(NULL==pIRQResStruct)
  {
   pIRQResStruct=&IRQResStruct;
   pIRQResStruct->ResourceType=RS_TYPE_IRQ;
   pIRQResStruct->IRQResource.IRQLevel=(unsigned short)irq;
   pIRQResStruct->IRQResource.PCIIrqPin=RS_PCI_INT_NONE;
   if(flags&SA_SHIRQ)
    pIRQResStruct->IRQResource.IRQFlags=RS_IRQ_SHARED;
   else
    pIRQResStruct->IRQResource.IRQFlags=0;
   pIRQResStruct->IRQResource.pfnIntHandler=0;
  }
  rc=RM32AllocResource(rm_resource->hDriver,&(rm_resource->hResource),pIRQResStruct);
  if(0==rc)
   rc=RM32ModifyResources(rm_resource->hDriver,rm_resource->hAdapterOrDevice
                          ,RM_MODIFY_ADD,rm_resource->hResource);
 }
 */
 return rc;
}

unsigned long LX_free_irq(struct lxrm_resource *rm_resource,unsigned int irq)
{
 unsigned long rc=0;
 /*
 if(!dolxdevtree || !dorm)
  return rc;
 if(rm_resource && (0!=rm_resource->hResource))
 {
  rc=RM32ModifyResources(rm_resource->hDriver,rm_resource->hAdapterOrDevice
                         ,RM_MODIFY_DELETE,rm_resource->hResource);
  if(0==rc)
  {
   rc=RM32DeallocResource(rm_resource->hDriver,rm_resource->hResource);
   if(0==rc)
    rm_resource->hAdapterOrDevice=0;
  }
 }
 */
 return rc;
}

//----------------------------- LX_request_region ------------------------------
unsigned long LX_request_region(struct lxrm_resource *rm_resource
                                 ,struct resource *a
                                 ,unsigned long start,unsigned long n)
{
 unsigned long rc=0;
 if(!dolxdevtree || !dorm)
  return rc;
 if(rm_resource && (0==rm_resource->hResource))
 {
  RESOURCESTRUCT MEMResStruct;
  PRESOURCESTRUCT pMEMResStruct=rm_resource->pResource;
  if(0==rm_resource->hDriver)
   rm_resource->hDriver=hDriver;
  if(0==rm_resource->hAdapterOrDevice)
   rm_resource->hAdapterOrDevice=hMEMAdap;
  if(NULL==pMEMResStruct)
  {
   pMEMResStruct=&MEMResStruct;
   pMEMResStruct->ResourceType=RS_TYPE_MEM;
   pMEMResStruct->RESOURCESTRUCTBODY.MEMResource.MemBase=start;
   pMEMResStruct->RESOURCESTRUCTBODY.MEMResource.MemSize=n;
   pMEMResStruct->RESOURCESTRUCTBODY.MEMResource.MemFlags=RS_MEM_EXCLUSIVE;
   pMEMResStruct->RESOURCESTRUCTBODY.MEMResource.ReservedAlign=0;
  }
  rc=RM32AllocResource(rm_resource->hDriver,&(rm_resource->hResource),pMEMResStruct);
  if(0==rc)
   rc=RM32ModifyResources(rm_resource->hDriver,rm_resource->hAdapterOrDevice
                          ,RM_MODIFY_ADD,rm_resource->hResource);
 }
 return rc;
}
unsigned long LX_release_region(struct lxrm_resource *rm_resource
                                 ,struct resource *a
                                 ,unsigned long start,unsigned long n)
{
 unsigned long rc=0;
 if(!dolxdevtree || !dorm)
  return rc;
 if(rm_resource && (0!=rm_resource->hResource))
 {
  rc=RM32ModifyResources(rm_resource->hDriver,rm_resource->hAdapterOrDevice
                         ,RM_MODIFY_DELETE,rm_resource->hResource);
  if(0==rc)
  {
   rc=RM32DeallocResource(rm_resource->hDriver,rm_resource->hResource);
   if(0==rc)
    rm_resource->hAdapterOrDevice=0;
  }
 }
 return rc;
}

extern WORD32 rm32_hDriver;
extern WORD32 rm32_hAdapter;
extern WORD32 rm32_hDevice;
extern WORD32 rm32_hResource;
extern WORD32 rm32_ModifyAction;
extern WORD32 rm32_DriverStruct;
extern WORD32 rm32_AdapterStruct;
extern WORD32 rm32_DeviceStruct;
extern WORD32 rm32_ResourceStruct;
extern WORD32 rm32_ResourceList;
extern WORD32 rm32_AdjunctBuffer;
extern WORD32 rm32_tmpStr1;
extern WORD32 rm32_tmpStr2;
extern WORD32 rm32_tmpStr3;

#define RM_GETLIN(rm32,rm32ptr) \
{ \
 if(!rm32ptr) \
  DevVirtToLin(SELECTOROF(rm32),OFFSETOF(rm32),(LINEAR*)&(rm32ptr)); \
}

//--------------------------- LX_initResourceManager ---------------------------
unsigned long LX_initResourceManager(void)
{
 unsigned long rc;
 {
  RM_GETLIN(rm32_hDriver,rm32_hDriverPtr);
  RM_GETLIN(rm32_hAdapter,rm32_hAdapterPtr);
  RM_GETLIN(rm32_hDevice,rm32_hDevicePtr);
  RM_GETLIN(rm32_hResource,rm32_hResourcePtr);
  RM_GETLIN(rm32_ModifyAction,rm32_ModifyActionPtr);
  RM_GETLIN(rm32_DriverStruct,rm32_DriverStructPtr);
  RM_GETLIN(rm32_AdapterStruct,rm32_AdapterStructPtr);
  RM_GETLIN(rm32_DeviceStruct,rm32_DeviceStructPtr);
  RM_GETLIN(rm32_ResourceStruct,rm32_ResourceStructPtr);
  RM_GETLIN(rm32_ResourceList,rm32_ResourceListPtr);
  RM_GETLIN(rm32_AdjunctBuffer,rm32_AdjunctBufferPtr);
  RM_GETLIN(rm32_tmpStr1,rm32_tmpStr1Ptr);
  RM_GETLIN(rm32_tmpStr2,rm32_tmpStr2Ptr);
  RM_GETLIN(rm32_tmpStr3,rm32_tmpStr3Ptr);
 }
 rc=0;
 if(dolxdevtree || !dorm)
 {
  DRIVERSTRUCT DriverStruct;
  LX_init_driver_struct(&DriverStruct);
  DriverStruct.DrvrDescript=(PSZ)"Linux API support layer";
  DriverStruct.DrvrType=DRT_SERVICE;
  DriverStruct.DrvrSubType=DRS_CONFIG;
  rc=RM32CreateDriver(&DriverStruct,&hDriver);
 }
 return rc;
}

