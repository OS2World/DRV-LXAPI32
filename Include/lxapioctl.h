/* $Id: lxapioctl.h,v 1.22 2005/07/23 21:15:13 smilcke Exp $ */

/*
 * lxapioctl.h
 * Autor:               Stefan Milcke
 * Erstellt am:         27.12.2001
 * Letzte Aenderung am: 21.07.2005
 *
*/

#ifndef LXAPIOCTL_H_INCLUDED
#define LXAPIIOCTL_H_INCLUDED

#pragma pack(1)

// Note: LX IOCtl categories starts at 0xA0
// Own LX IOCtl functions should be between 0x00 and 0x9F
// All unsupported IOCtl categories above 0x9F should be routed to LX_lxioctl()
#define LXIOCCAT_DEH       0xA0     // Category for OS/2 device helper calls
#define LXIOCCAT_GLO       0xA1     // Category for global calls
#define LXIOCCAT_MOD       0xA2     // Category for module calls
#define LXIOCCAT_PCI       0xA3     // Category for pci calls
#define LXIOCCAT_I2C       0xA4     // Category for i2c calls
#define LXIOCCAT_MEM       0xA5     // Category for memory function calls

#define LXIOCCAT_V4L       0xB0     // Category for V4L (video for linux) calls

#define LXIOCCAT_LFS       0xC0     // Category for special fs calls

#define LXAUTOFREE_ALLWAYS                0
#define LXAUTOFREE_ON_THIS_CLOSE_DEVICE   1
#define LXAUTOFREE_ON_LAST_CLOSE_DEVICE   2
#define LXAUTOFREE_NEVER                  3

/******************************************************************************/
/* OS/2 device helper ioctl functions                                         */
/******************************************************************************/
#define LXIOCFN_DEH_ALLOCPHYS                               0x01
typedef struct _LXIOCPA_DEH_ALLOCPHYS
{
 INT rc;
 ULONG size;
 ULONG high_or_low;
 VOID *pMem;
} LXIOCPA_DEH_ALLOCPHYS,*PLXIOCPA_DEH_ALLOCPHYS;

#define LXIOCFN_DEH_FREEPHYS                                0x02
typedef struct _LXIOCPA_DEH_FREEPHYS
{
 INT rc;
 VOID *pMem;
} LXIOCPA_DEH_FREEPHYS,*PLXIOCPA_DEH_FREEPHYS;

#define LXIOCFN_DEH_VMALLOC                                 0x03
typedef struct _LXIOCPA_DEH_VMALLOC
{
 INT rc;
 ULONG flags;
 ULONG size;
 VOID *pPhysAddr;
 VOID *pMem;
 SHORT sel;
} LXIOCPA_DEH_VMALLOC,*PLXIOCPA_DEH_VMALLOC;

#define LXIOCFN_DEH_VMFREE                                  0x04
typedef struct _LXIOCPA_DEH_VMFREE
{
 INT rc;
 VOID *pMem;
} LXIOCPA_DEH_VMFREE,*PLXIOCPA_DEH_VMFREE;

#define LXIOCFN_DEH_VMGLOBALTOPROCESS                       0x05
typedef struct _LXIOCPA_DEH_VMGLOBALTOPROCESS
{
 INT rc;
 ULONG flags;
 VOID *pLinearAddr;
 ULONG size;
 VOID *pMem;
} LXIOCPA_DEH_VMGLOBALTOPROCESS,*PLXIOCPA_DEH_VMGLOBALTOPROCESS;

#define LXIOCFN_DEH_VMPROCESSTOGLOBAL                       0x06
typedef struct _LXIOCPA_DEH_VMPROCESSTOGLOBAL
{
 INT rc;
 ULONG flags;
 VOID *pLinearAddr;
 ULONG size;
 VOID *pMem;
} LXIOCPA_DEH_VMPROCESSTOGLOBAL,*PLXIOCPA_DEH_VMPROCESSTOGLOBAL;

typedef struct
{
 UCHAR lock[12];
}VMLock;

typedef struct
{
 ULONG addr;
 ULONG size;
}VMPageList;

#define LXIOCFN_DEH_VMLOCK                                  0x07
typedef struct _LXIOCPA_DEH_VMLOCK
{
 INT rc;
 ULONG flags;
 VOID *pLinearAddr;
 ULONG size;
 VMLock *lock;
 ULONG pagelistSize;
 VMPageList *pPageList;
} LXIOCPA_DEH_VMLOCK,*PLXIOCPA_DEH_VMLOCK;

#define LXIOCFN_DEH_VMUNLOCK                                0x08
typedef struct _LXIOCPA_DEH_VMUNLOCK
{
 INT rc;
 VMLock *lock;
} LXIOCPA_DEH_VMUNLOCK,*PLXIOCPA_DEH_VMUNLOCK;

#define LXIOCFN_DEH_VDHALLOCPAGES                           0xa0
#define VDHAP_SPECIFIC                                      0x0001
#define VDHAP_SYSTEM                                        0x0002
#define VDHAP_FIXED                                         0x0004
#define VDHAP_SWAPPABLE                                     0x0000
#define VDHAP_PHYSICAL                                      0x0008
typedef struct _LXIOCPA_DEH_VDHALLOCPAGES
{
 ULONG ObjectAddress;
 PVOID StartingAddress;
 ULONG NumPages;
 ULONG OptionFlag;
} LXIOCPA_DEH_VDHALLOCPAGES,*PLXIOCPA_DEH_VDHALLOCPAGES;

#define LXIOCFN_DEH_VDHFREEPAGES                            0xa1
typedef struct _LXIOCPA_DEH_VDHFREEPAGES
{
 ULONG ObjectAddress;
}LXIOCPA_DEH_VDHFREEPAGES,*PLXIOCPA_DEH_VDHFREEPAGES;

/******************************************************************************/
/* global ioctl functions                                                     */
/******************************************************************************/
#define LXIOCFN_GLO_GETDRIVERINFO                           0x01
typedef struct _LXIOCDP_GLO_GETDRIVERINFO
{
 ULONG ulVersionMajor;
 ULONG ulVersionMinor;
 ULONG ulBuildLevel;
 CHAR vendor[32];
 ULONG ulNumModules;
 ULONG ulV4LXNumDevices;
} LXIOCDP_GLO_GETDRIVERINFO,*PLXIOCDP_GLO_GETDRIVERINFO;

#define LXIOCFN_GLO_GETSYSFILENUM                           0x02
typedef struct _LXIOCDP_GLO_GETSYSFILENUM
{
 ULONG ulSysFileNum;
} LXIOCDP_GLO_GETSYSFILENUM,*PLXIOCDP_GLO_GETSYSFILENUM;

/******************************************************************************/
/* module ioctl functions                                                     */
/******************************************************************************/
#define LXIOCFN_MOD_ENUMMODULES                             0x01
typedef struct _LX_MODINFO
{
 CHAR name[32];
 ULONG versioncode;
 INT active;
}LX_MODINFO,*PLX_MODINFO;
typedef struct _LXIOCDP_MOD_ENUMMODULES
{
 ULONG ulNumModules;
 LX_MODINFO *modules;
}LXIOCDP_MOD_ENUMMODULES,*PLXIOCDP_MOD_ENUMMODULES;

#define LXIOCFN_MOD_REQUEST_MODULE                          0x02
typedef struct _LXIOCPA_MOD_REQUEST_MODULE
{
 INT rc;
 CHAR name[32];
}LXIOCPA_MOD_REQUEST_MODULE,*PLXIOCPA_MOD_REQUEST_MODULE;

#define LXIOCFN_MOD_RELEASE_MODULE                          0x03
typedef struct _LXIOCPA_MOD_RELEASE_MODULE
{
 INT rc;
 CHAR name[32];
}LXIOCPA_MOD_RELEASE_MODULE,*PLXIOCPA_MOD_RELEASE_MODULE;

#define LXIOCFN_MOD_SETMODPARM                              0x04
typedef struct _LXIOCPA_MOD_SETPARM
{
 INT rc;
 CHAR moduleName[32];
 CHAR *parameter;
}LXIOCPA_MOD_SETPARM,*PLXIOCPA_MOD_SETPARM;

#define LXIOCFN_MOD_ENUMMODPARMS                            0x05
typedef struct _LX_MODPARM
{
 CHAR name[32];
 CHAR value[224];
}LX_MODPARM,*PLX_MODPARM;
typedef struct _LXIOCPA_MOD_ENUMPARMS
{
 INT rc;
 ULONG ulParmSize;
 CHAR name[32];
}LXIOCPA_MOD_ENUMPARMS,*PLXIOCPA_MOD_ENUMPARMS;
typedef struct _LXIOCDP_MOD_ENUMPARM
{
 ULONG ulNumParms;
 LX_MODPARM* parms;
}LXIOCDP_MOD_ENUMPARMS,*PLXIOCDP_MOD_ENUMPARMS;

/******************************************************************************/
/* memory ioctl functions                                                     */
/******************************************************************************/
#define LXIOCFN_MEM_VMALLOC                                 0x01
typedef struct _LXIOCPA_MEM_VMALLOC
{
 INT rc;
 ULONG size;
 ULONG autofree_flags;
}LXIOCPA_MEM_VMALLOC,*PLXIOCPA_MEM_VMALLOC;
typedef struct _LXIOCDP_MEM_VMALLOC
{
 VOID *kernelMem;
 VOID *physMem;
 VOID *virtMem;
}LXIOCDP_MEM_VMALLOC,*PLXIOCDP_MEM_VMALLOC;

#define LXIOCFN_MEM_VFREE                                   0x02
typedef struct _LXIOCPA_MEM_VFREE
{
 INT rc;
 VOID *kernelMem;
}LXIOCPA_MEM_VFREE,*PLXIOCPA_MEM_VFREE;

#define LXIOCFN_MEM_MAPTOPHYS                               0x03
typedef struct _LXIOCPA_MEM_MAPTOPHYS
{
 INT rc;
 ULONG size;
 ULONG autofree_flags;
 VOID *virtMem;
}LXIOCPA_MEM_MAPTOPHYS,*PLXIOCPA_MEM_MAPTOPHYS;

typedef struct _LXIOCDP_MEM_MAPTOPHYS
{
 VOID *physMem;
}LXIOCDP_MEM_MAPTOPHYS,*PLXIOCDP_MEM_MAPTOPHYS;

#define LXIOCFN_MEM_UNMAPTOPHYS                             0x04
typedef struct _LXIOCPA_MEM_UNMAPTOPHYS
{
 INT rc;
 VOID *virtMem;
 VOID *physMem;
}LXIOCPA_MEM_UNMAPTOPHYS,*PLXIOCPA_MEM_UNMAPTOPHYS;

#define LXIOCFN_MEM_ALLOCPHYS                               0x05
typedef struct _LXIOCPA_MEM_ALLOCPHYS
{
 INT rc;
 ULONG size;
}LXIOCPA_MEM_ALLOCPHYS,*PLXIOCPA_MEM_ALLOCPHYS;
typedef struct _LXIOCDP_MEM_ALLOCPHYS
{
 VOID *physMem;
}LXIOCDP_MEM_ALLOCPHYS,*PLXIOCDP_MEM_ALLOCPHYS;

#define LXIOCFN_MEM_FREEPHYS                                0x06
typedef struct _LXIOCPA_MEM_FREEPHYS
{
 INT rc;
 VOID *physMem;
}LXIOCPA_MEM_FREEPHYS,*PLXIOCPA_MEM_FREEPHYS;

/******************************************************************************/
/* V4L ioctl functions                                                        */
/******************************************************************************/
#define LXIOCFN_V4L_ENUMDEVICES                             0x01
typedef struct _LXV4L_DEVICEINFO
{
 CHAR name[32];
 CHAR devname[16];
 ULONG type;
 ULONG hardware;
}LX_V4L_DEVICEINFO,*PLX_V4L_DEVICEINFO;
typedef struct _LXIOCDP_V4L_ENUMDEVICES
{
 ULONG ulDevices;
 LX_V4L_DEVICEINFO *devInfos;
}LXIOCDP_V4L_ENUMDEVICES,*PLXIOCDP_V4L_ENUMDEVICES;

#define LXIOCFN_V4L_OPENDEVICE                              0x02
typedef struct _LXIOCPA_V4L_OPENDEVICE
{
 INT rc;
 INT handle;
 ULONG autofree_flags;
 CHAR devname[32];
}LXIOCPA_V4L_OPENDEVICE,*PLXIOCPA_V4L_OPENDEVICE;

#define LXIOCFN_V4L_CLOSEDEVICE                             0x03
typedef struct _LXIOCPA_V4L_CLOSEDEVICE
{
 INT rc;
 INT handle;
}LXIOCPA_V4L_CLOSEDEVICE,*PLXIOCPA_V4L_CLOSEDEVICE;

#define LXIOCFN_V4L_IOCTL                                   0x04
typedef struct _LXIOCPA_V4L_IOCTL
{
 INT rc;
 INT handle;
 INT v4l_ioctlfn;
}LXIOCPA_V4L_IOCTL,*PLXIOCPA_V4L_IOCTL;
typedef struct _LXIOCDP_V4L_IOCTL
{
 VOID *userData;
}LXIOCDP_V4L_IOCTL,*PLXIOCDP_V4L_IOCTL;

#define LXIOCFN_V4L_MMAP                                    0x05
typedef struct _LXIOCPA_V4L_MMAP
{
 INT rc;
 INT handle;
 INT size;
 INT flags;
}LXIOCPA_V4L_MMAP,*PLXIOCPA_V4L_MMAP;
typedef struct _LXIOCDP_V4L_MMAP
{
 VOID *mmap;
}LXIOCDP_V4L_MMAP,*PLXIOCDP_V4L_MMAP;

#define LXIOCFN_V4L_READ                                    0x06
typedef struct _LXIOCPA_V4L_READ
{
 INT rc;
 INT handle;
 INT count;
 INT nonblock;
}LXIOCPA_V4L_READ,*PLXIOCPA_V4L_READ;
typedef struct _LXIOCDP_V4L_READ
{
 VOID *buffer;
}LXIOCDP_V4L_READ,*PLXIOCDP_V4L_READ;

#define LXIOCFN_V4L_POLL                                    0x07

#pragma pack()
#endif
