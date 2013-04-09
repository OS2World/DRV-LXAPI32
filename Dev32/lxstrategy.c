/* $Id: lxstrategy.c,v 1.47 2006/01/05 23:48:17 smilcke Exp $ */

/*
 * lxstrategy.c
 * Autor:               Stefan Milcke
 * Erstellt am:         07.09.2001
 * Letzte Aenderung am: 29.12.2005
 *
*/

#include <lxcommon.h>
#include <DEVRP.H>

#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/module.h>

#include <lxapi.h>
#include <linux/string.h>
#include <lxapioctl.h>
#include <lxdaemon.h>

ULONG LX_StratIOCtl(struct LX_RP* prp);
WORD32 numOS2Opens = 0;

unsigned short current_fileId=0;
unsigned long current_autofree_flags=0;
char* current_devName=NULL;

PLXAPI_DEVICE lxapi_root_device=NULL;
static spinlock_t lxapidevlock=SPIN_LOCK_UNLOCKED;

static char devFileName[]="lxapi32.sys";

//----------------------- LX_get_lxapidevice_from_fileId -----------------------
PLXAPI_DEVICE LX_get_lxapidevice_from_fileId(unsigned short fileId)
{
 PLXAPI_DEVICE pDev=lxapi_root_device;
 while(pDev && pDev->fileId!=fileId)
  pDev=pDev->next;
 return pDev;
}

EXPORT_SYMBOL(LX_get_lxapidevice_from_fileId);

//-------------------- LX_get_other_lxapidevice_from_fileId --------------------
PLXAPI_DEVICE LX_get_other_lxapidevice_from_fileId(unsigned short fileId)
{
 PLXAPI_DEVICE pDev=LX_get_lxapidevice_from_fileId(fileId);
 if(pDev)
 {
  PLXAPI_DEVICE pOther=lxapi_root_device;
  while(pOther)
  {
   if(pOther!=pDev && !strcmp(pOther->name,pDev->name))
     break;
   pOther=pOther->next;
  }
  return pOther;
 }
 else
  return NULL;
}

EXPORT_SYMBOL(LX_get_other_lxapidevice_from_fileId);

//------------------------- LX_set_current_dev_fileId --------------------------
void LX_set_current_dev_fileId(unsigned short fileId)
{
 PLXAPI_DEVICE p=NULL;
 if(fileId)
  p=LX_get_lxapidevice_from_fileId(fileId);
 current_fileId=fileId;
 if(p)
 {
  current_devName=p->name;
  current_autofree_flags=LXAUTOFREE_ON_THIS_CLOSE_DEVICE;
 }
 else
 {
  current_devName=NULL;
  current_autofree_flags=LXAUTOFREE_ON_THIS_CLOSE_DEVICE;
 }
}

EXPORT_SYMBOL(LX_set_current_dev_fileId);

//--------------------------- LX_open_device_called ----------------------------
int LX_open_device_called(unsigned short fileId,char *devName)
{
 PLXAPI_DEVICE pDev=(PLXAPI_DEVICE)kmalloc(sizeof(LXAPI_DEVICE),GFP_KERNEL);
 pDev->name=devName;
 pDev->fileId=fileId;
 spin_lock_irq(&lxapidevlock);
 pDev->next=lxapi_root_device;
 lxapi_root_device=pDev;
 spin_unlock_irq(&lxapidevlock);
 current_fileId=0;
 current;  // Force a current
 return 0;
}

EXPORT_SYMBOL(LX_open_device_called);

//--------------------------- LX_close_device_called ---------------------------
int LX_close_device_called(unsigned short fileId)
{
 PLXAPI_DEVICE pDev=lxapi_root_device;
 PLXAPI_DEVICE pPrev=NULL;
 LX_set_current_dev_fileId(fileId);
 LX_v4lx_close_file_related_devices(fileId);
// LX_free_file_related_mappings(fileId);
// LX_free_file_related_vm(fileId);
 spin_lock_irq(&lxapidevlock);
 pDev=lxapi_root_device;
 while(pDev && pDev->fileId!=fileId)
 {
  pPrev=pDev;
  pDev=pDev->next;
 }
 if(pDev)
 {
  if(pDev==lxapi_root_device)
   lxapi_root_device=lxapi_root_device->next;
  else
  {
   if(pPrev)
    pPrev->next=pDev->next;
  }
  kfree(pDev);
 }
 spin_unlock_irq(&lxapidevlock);
 current_fileId=0;
 return 0;
}

EXPORT_SYMBOL(LX_close_device_called);

//-------------------------------- LX_StratOpen --------------------------------
WORD32 LX_StratOpen(struct LX_RP* rp)
{
 LX_open_device_called(rp->RPBODY.RPOPENCLOSE.FileID,devFileName);
 numOS2Opens++;
 return RPDONE;
}

//------------------------------- LX_StratClose --------------------------------
ULONG LX_StratClose(struct LX_RP* rp)
{
 numOS2Opens--;
 if(lx_daemon_pid==lx_current_pid)
  LX_daemon_closed();
 LX_close_device_called(rp->RPBODY.RPOPENCLOSE.FileID);
 return(RPDONE);
}

//------------------------------- LX_StratWrite --------------------------------
static WORD32 LX_StratWrite(struct LX_RP* rp)
{
  return RPDONE | RPERR;
}

extern WORD32 LX_StratInit(struct LX_RP* rp);

extern asmlinkage long sys_sync(void);
extern void LX_InitComplete_Daemon(void);
extern void LX_InitComplete_Memory(void);
extern int lx_startup_ok;
//---------------------------- LX_StratInitComplete ----------------------------
WORD32 LX_StratInitComplete(struct LX_RP* rp)
{
 if(lx_startup_ok)
 {
  if((lx_sysstate&LXSYSSTATE_SYSTEM_RUNNING))
  {
   LX_InitComplete_Memory(); // must be the first !!
   LX_InitComplete_Daemon();
  }
 }
 printk("OS2: InitComplete reached\n");
 return(RPDONE);
}

//------------------------------- LX_StratError --------------------------------
static WORD32 LX_StratError(struct LX_RP* rp)
{
  return RPERR_COMMAND | RPDONE;
}

extern void do_cleanup_all_requested_modules(int external);
extern void shutdown_all_kernel_threads(void);

//----------------------------- LX_StratDeinstall ------------------------------
static WORD32 LX_StratDeinstall(struct LX_RP* rp)
{
// do_cleanup_all_requested_modules(1);
// do_cleanup_all_requested_modules(0);
 atomic_set(&lx_exit_kernel,1);
 return RPDONE;
}

extern unsigned long LX_daemon_shutdown(void);
extern void fastcall LXA_DisableIrqs(void);
//------------------------------ LX_StratShutdown ------------------------------
static WORD32 LX_StratShutdown(struct LX_RP* rp)
{
/*
 if(rp->RPSHUTDOWN.Function==0)
  do_cleanup_all_requested_modules(1);    // Unload external modules
 else
 {
  do_cleanup_all_requested_modules(0);    // Unload internal modules
  shutdown_all_kernel_threads();
 }
*/
 if(rp->RPBODY.RPSHUTDOWN.Function==0)
 {
  lxa_shutdown_level=1;
  sys_sync();
  atomic_set(&lx_exit_kernel,1);
  if(lx_startup_ok)
   wake_up_all(&lx_shutdown_wq);
 }
 else
 {
  lxa_shutdown_level=2;
  if(lx_startup_ok)
  {
   wake_up_all(&lx_shutdown_wq);
   LXA_DisableIrqs();
  }
//  LX_daemon_shutdown();
 }
 return RPDONE;
}

// Strategy dispatch table
//
// This table is used by the strategy routine to dispatch strategy requests

typedef WORD32 (*RPHandler)(struct LX_RP* rp);
static RPHandler lx_StratDispatch[] =
{
  LX_StratInit,         // 00 (BC): Initialization
  LX_StratError,        // 01 (B ): Media check
  LX_StratError,        // 02 (B ): Build BIOS parameter block
  LX_StratError,        // 03 (  ): Unused
  LX_StratError,        // 04 (BC): Read
  LX_StratError,        // 05 ( C): Nondestructive read with no wait
  LX_StratError,        // 06 ( C): Input status
  LX_StratError,        // 07 ( C): Input flush
  LX_StratWrite,        // 08 (BC): Write
  LX_StratError,        // 09 (BC): Write verify
  LX_StratError,        // 0A ( C): Output status
  LX_StratError,        // 0B ( C): Output flush
  LX_StratError,        // 0C (  ): Unused
  LX_StratOpen,         // 0D (BC): Open
  LX_StratClose,        // 0E (BC): Close
  LX_StratError,        // 0F (B ): Removable media check
  LX_StratIOCtl,        // 10 (BC): IO Control
  LX_StratError,        // 11 (B ): Reset media
  LX_StratError,        // 12 (B ): Get logical unit
  LX_StratError,        // 13 (B ): Set logical unit
  LX_StratDeinstall,    // 14 ( C): Deinstall character device driver
  LX_StratError,        // 15 (  ): Unused
  LX_StratError,        // 16 (B ): Count partitionable fixed disks
  LX_StratError,        // 17 (B ): Get logical unit mapping of fixed disk
  LX_StratError,        // 18 (  ): Unused
  LX_StratError,        // 19 (  ): Unused
  LX_StratError,        // 1A (  ): Unused
  LX_StratError,        // 1B (  ): Unused
  LX_StratShutdown,     // 1C (BC): Notify start or end of system shutdown
  LX_StratError,        // 1D (B ): Get driver capabilities
  LX_StratError,        // 1E (  ): Unused
  LX_StratInitComplete  // 1F (BC): Notify end of initialization
};

//-------------------------------- LX_Strategy ---------------------------------
WORD32 fastcall LX_Strategy(struct LX_RP* rp)
{
 WORD32 rc;
 if (rp->Command < sizeof(lx_StratDispatch)/sizeof(lx_StratDispatch[0]))
 {
  LX_enter_current();
  rc=(lx_StratDispatch[rp->Command](rp));
  LX_leave_current();
 }
 else
  rc=(RPERR_COMMAND | RPDONE);
 return rc;
}
