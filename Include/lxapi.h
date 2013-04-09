/* $Id: lxapi.h,v 1.23 2006/02/16 23:07:16 smilcke Exp $ */

/*
 * lxapi.h
 * Autor:               Stefan Milcke
 * Erstellt am:         05.09.2002
 * Letzte Aenderung am: 23.04.2005
 *
*/

#ifndef LXAPI_H_INCLUDED
#define LXAPI_H_INCLUDED

#include <devhlp32.h>

#include <linux/list.h>
#include <linux/module.h>

#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/atomic.h>

#include <stdio.h>

/* IRQ Management */
extern int LX_timer_started(void);
extern void LX_set_irq_handled(int irq);
extern int LX_irq_handled(int irq);
extern int LX_RequestIrq(int irq,int flags);
extern void LX_FreeIrq(int irq);

/* Early file IO */
int LX_si_open(char* sFileName,unsigned long* pHandle
               ,unsigned long ulOpenFlag,unsigned long ulOpenMode);
int LX_si_close(unsigned long handle);
int LX_si_read(unsigned long handle,unsigned long* pcbBytes,void* pBuffer);
char* LX_si_linein(unsigned long handle,unsigned long* pcbBytes,char* pBuffer);


/* The SYMPARM segment is a special wrapper for exported symbols and parameters
   in the OS/2 version of GCC.
*/

struct lx_SYMPARM_segment
{
 /* list */
 struct list_head list;
 /* start and length of segment */
 unsigned long start;
 unsigned long len;
 int modparm_count;
 void* modparms;
 int kernel_symbol_count;
 void* kernel_symbols;
};

extern struct obsolete_modparm* _LX_find_modparm(const char* symname
                                                 ,struct obsolete_modparm* pModParm
                                                 ,int doExact);

void LX_process_script(char* scrname);

typedef struct _LXAPI_DEVICE
{
 struct _LXAPI_DEVICE* next;
 unsigned short fileId;
 char* name;
}LXAPI_DEVICE,*PLXAPI_DEVICE;
extern PLXAPI_DEVICE lxapi_root_device;
extern unsigned short current_fileId;
extern unsigned long current_autofree_flags;
extern char *current_devName;

int LX_lock_mem(unsigned long addr,unsigned long size,unsigned long flags);
int LX_unlock_mem(unsigned long addr);
void LX_set_current_dev_fileId(unsigned short fileId);
PLXAPI_DEVICE LX_get_lxapidevice_from_fileId(unsigned short fileId);
PLXAPI_DEVICE LX_get_other_lxapidevice_from_fileId(unsigned short fileId);
void LX_v4lx_close_file_related_devices(unsigned short fileId);
int LX_remove_current_process_from_list(void);

typedef struct _VMVIRTTOPHYSMAPPING
{
 struct _VMVIRTTOPHYSMAPPING* next;
 unsigned short fileId;
 unsigned long autofree_flags;
 unsigned long size;
 void *virtAddr;
 unsigned long flags;
 char lock[12];
 PAGELIST page;
}VMVIRTTOPHYSMAPPING,*PVMVIRTTOPHYSMAPPING;
extern PVMVIRTTOPHYSMAPPING virttophys_root;

typedef struct _VMMEMADDR
{
 struct _VMMEMADDR *next;
 unsigned short fileId;
 unsigned long autofree_flags;
 unsigned long size;
 unsigned long mem_flags;
 void *pVirtAddr;
 void *pPhysAddr;
} VMMEMADDR,*PVMMEMADDR;
PVMMEMADDR LX_vmalloc(unsigned long size
                       ,unsigned long mem_flags
                       ,unsigned long free_flags);

extern PVMMEMADDR vm_root_addr;

#endif //LXAPI_H_INCLUDED
