/* $Id: lxrmcall.h,v 1.5 2006/01/05 23:48:20 smilcke Exp $ */

/*
 * lxrmcall.h
 * Autor:               Stefan Milcke
 * Erstellt am:         12.03.2002
 * Letzte Aenderung am: 29.12.2005
 *
*/

#ifndef LXRMCALL_H_INCLUDED
#define LXRMCALL_H_INCLUDED

/* defines for OS/2 include */

/* Standard headers */

/* Application headers */

/* OS/2 Headers */

#ifndef NEAR
# define NEAR
#endif
#ifndef __32BIT__
#define __32BIT__
#endif
#ifndef _Optlink
#define _Optlink
#endif
#include <rmcalls.h>

#ifdef TARGET_OS2_GNU2
#define __cdecl
#endif

typedef unsigned long RM32RET;

struct _RM_ADAPTER
{
 char *name;
 unsigned long rm_hDriver;
 unsigned long rm_hAdapter;
 unsigned short rm_BaseType;
 unsigned short rm_SubType;
 unsigned short rm_InterfaceType;
 unsigned short rm_HostBusType;
 unsigned short rm_HostBusWidth;
};

struct lxrm_driver
{
 unsigned long hDriver;
 DRIVERSTRUCT *pDriver;
};

struct lxrm_adapter
{
 unsigned long hDriver;
 unsigned long hAdapter;
 ADAPTERSTRUCT *pAdapter;
};

struct lxrm_subadapter
{
 unsigned long hDriver;
 unsigned long hParentDevice;
 unsigned long hAdapter;
 ADAPTERSTRUCT *pAdapter;
};

struct lxrm_device
{
 unsigned long hDriver;
 unsigned long hAdapter;
 unsigned long hDevice;
 DEVICESTRUCT *pDevice;
};

struct lxrm_subdevice
{
 unsigned long hDriver;
 unsigned long hParentAdapter;
 unsigned long hDevice;
 DEVICESTRUCT *pDevice;
};

struct lxrm_resource
{
 unsigned long hDriver;
 unsigned long hAdapterOrDevice;
 unsigned long hResource;
 RESOURCESTRUCT *pResource;
};

RM32RET __cdecl RM32CreateDriver(DRIVERSTRUCT*,HDRIVER*);
RM32RET __cdecl RM32DestroyDriver(HDRIVER);
RM32RET __cdecl RM32CreateAdapter(HDRIVER,HADAPTER*,ADAPTERSTRUCT*,HDEVICE,AHRESOURCE*);
RM32RET __cdecl RM32DestroyAdapter(HDRIVER,HADAPTER);
RM32RET __cdecl RM32CreateDevice(HDRIVER,HDEVICE*,DEVICESTRUCT*,HADAPTER,AHRESOURCE*);
RM32RET __cdecl RM32DestroyDevice(HDRIVER,HDEVICE);
RM32RET __cdecl RM32AllocResource(HDRIVER,HRESOURCE*,RESOURCESTRUCT*);
RM32RET __cdecl RM32DeallocResource(HDRIVER,HRESOURCE);
/*
RM32RET __cdecl RM32CreateLDev(HDRIVER,HLDEV*,HDEVICE,LDEVSTRUCT*);
RM32RET __cdecl RM32DestroyLDev(HDRIVER,HLDEV);
RM32RET __cdecl RM32CreateSysName(HDRIVER,HSYSNAME*,HLDEV,SYSNAMESTRUCT*);
RM32RET __cdecl RM32DestroySysName(HDRIVER,HSYSNAME);
RM32RET __cdecl RM32ADDToHDEVICE(HDEVICE*,USHORT,USHORT);
RM32RET __cdecl RM32KeyToHandleList(RMHANDLE,PSZ,HANDLELIST*);
RM32RET __cdecl RM32HandleToType(RMHANDLE,USHORT*);
RM32RET __cdecl RM32HandleToParent(RMHANDLE,RMHANDLE*);
*/
RM32RET __cdecl RM32ModifyResources(HDRIVER,HADAPTER,USHORT,HRESOURCE);
/*
RM32RET __cdecl RM32ParseScsiInquiry(VOID*,PSZ,USHORT);
RM32RET __cdecl RM32UpdateAdjunct(HDRIVER,HDEVICE,USHORT,ADJUNCT*);
RM32RET __cdecl RM32AdjToHandleList(ADJUNCT*,HADAPTER,ADJHANDLELIST*);
RM32RET __cdecl RM32HDevToHLDev(HDEVICE,HLDEV,HLDEV*);
RM32RET __cdecl RM32ResToHandleList(RESOURCESTRUCT*,HANDLELIST*);
RM32RET __cdecl RM32GetNodeInfo(RMHANDLE,RM_GETNODE_DATA*,USHORT);
RM32RET __cdecl RM32CreateDetected(HDRIVER,HDETECTED*,DETECTEDSTRUCT*,AHRESOURCE*);
RM32RET __cdecl RM32DestroyDetected(HDRIVER,HDETECTED*);
RM32RET __cdecl RM32DevIDToHandleList(IDTYPE,DEVID,DEVID,DEVID,SEARCHIDFLAGS,HDETECTED,HANDLELIST*);
RM32RET __cdecl RM32HandleToResourceHandleList(RMHANDLE,HANDLELIST*);
RM32RET __cdecl RM32ModifyNodeFlags(HDRIVER,RMHANDLE,USHORT);
RM32RET __cdecl RM32ConvertID(ULONG*,PSZ,USHORT);
RM32RET __cdecl RM32GetCommandLine(PSZ,PSZ,USHORT*,USHORT,USHORT);
RM32RET __cdecl RM32GetVersion(USHORT*,USHORT*);
RM32RET __cdecl RM32SetSnoopLevel(USHORT,USHORT);
RM32RET __cdecl RM32SaveDetectedData(USHORT);
RM32RET __cdecl RM32DeleteDetectedData(USHORT);
*/

#endif //LXRMCALL_H_INCLUDED
