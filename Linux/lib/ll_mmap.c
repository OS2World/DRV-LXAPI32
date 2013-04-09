/* $Id: ll_mmap.c,v 1.4 2005/03/22 22:34:33 smilcke Exp $ */

/*
 * mmap.c
 * Autor:               Stefan Milcke
 * Erstellt am:         13.05.2002
 * Letzte Aenderung am: 16.09.2004
 *
*/

#include <lxcommon.h>

#include <asm/bitops.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <asm/page.h>
#include <linux/videodev.h>
#include <asm/io.h>
#include <lxapi.h>

int mmap_handle=1;

struct vm_mapped_mem
{
 struct vm_mapped_mem* next;
 unsigned long handle;  // map handle
 unsigned short pid;    // process id
 unsigned long size;    // size of memory object
 void *vAddr;           // virtual address
 void *pAddr;           // physical address
 void *mAddr;           // mapped address
 SHORT mSel;            // Selector from VMAlloc
};

struct vm_mapped_mem* vm_mapped_mem_root=NULL;

extern struct video_device *LX_v4lx_get_opened_device(int handle);
static spinlock_t mmap_lock=SPIN_LOCK_UNLOCKED;

//---------------------------- LX_remap_page_range -----------------------------
int LX_remap_page_range(struct vm_area_struct* vma,
                     unsigned long from,unsigned long to,
                     unsigned long size,pgprot_t prot)
{
 int ret=-EINVAL;
 struct vm_mapped_mem* pMap=NULL;
 // Try to find a previous mapping and reuse it
 {
  spin_lock_irq(&mmap_lock);
  pMap=vm_mapped_mem_root;
  while(pMap)
  {
   if(pMap->vAddr==(void*)to && pMap->size==size)
   {
    pMap->handle=from;
    ret=0;
    break;
   }
   pMap=pMap->next;
  }
  spin_unlock_irq(&mmap_lock);
 }
 if(!pMap)
 { // Create a new mapping
  pMap=(struct vm_mapped_mem*)kmalloc(sizeof(struct vm_mapped_mem),GFP_KERNEL);
  if(pMap)
  {
   pMap->handle=from;
   pMap->pid=lx_current_pid;
   pMap->size=size;
   pMap->vAddr=(void *)to;
   {
    pMap->pAddr=(void *)virt_to_phys((void *)to);
   }
   if(DevVMAlloc(VMDHA_PHYS | VMDHA_PROCESS,size
                 ,(LINEAR)&(pMap->pAddr),(LINEAR*)&(pMap->mAddr))
    && DevVMAlloc(VMDHA_PHYS | VMDHA_PROCESS | VMDHA_16M,size
                  ,(LINEAR)&(pMap->pAddr),(LINEAR*)&(pMap->mAddr)))
    DevVMFree((LINEAR)pMap);
   else
   {
    spin_lock_irq(&mmap_lock);
    if(NULL==vm_mapped_mem_root)
    {
     pMap->next=NULL;
     vm_mapped_mem_root=pMap;
    }
    else
    {
     pMap->next=vm_mapped_mem_root;
     vm_mapped_mem_root=pMap;
    }
    spin_unlock_irq(&mmap_lock);
    ret=0;
   }
  }
 }
 return ret;
}

//----------------------------- LX_get_mapped_mem ------------------------------
struct vm_mapped_mem* LX_get_mapped_mem(unsigned long handle)
{
 struct vm_mapped_mem* pMap=NULL;
 spin_lock_irq(&mmap_lock);
 pMap=vm_mapped_mem_root;
 while(pMap && pMap->handle!=handle && pMap->pid!=lx_current_pid)
  pMap=pMap->next;
 spin_unlock_irq(&mmap_lock);
 return pMap;
}

//---------------------------- LX_free_vm_mappings -----------------------------
int LX_free_vm_mappings(void *vAddr)
{
 struct vm_mapped_mem* pMap=NULL;
 struct vm_mapped_mem* pPrev=NULL;
 spin_lock_irq(&mmap_lock);
 pMap=vm_mapped_mem_root;
 while(pMap)
 {
  if(vAddr==pMap->vAddr)
  {
   DevVMFree((LINEAR)pMap->mAddr);
   if(pPrev)
    pPrev->next=pMap->next;
   if(vm_mapped_mem_root==pMap)
    vm_mapped_mem_root=pMap->next;
   DevVMFree((LINEAR)pMap);
   pMap=vm_mapped_mem_root;
  }
  else
  {
   pPrev=pMap;
   pMap=pMap->next;
  }
 }
 spin_unlock_irq(&mmap_lock);
 return 0;
}

//----------------------------- LX_map_vm_to_user ------------------------------
void* LX_map_vm_to_user(void *vPtr,unsigned long size)
{
 struct vm_mapped_mem *pMap=NULL;
 pgprot_t prot={0};
 unsigned long mhandle;
 spin_lock_irq(&mmap_lock);
 mhandle=mmap_handle++;
 if(mmap_handle>255)
  mmap_handle=1;
 spin_unlock_irq(&mmap_lock);
 if(!remap_page_range(0,mhandle,(unsigned long)vPtr,size,prot))
 {
  pMap=LX_get_mapped_mem(mhandle);
  if(pMap)
   return pMap->mAddr;
 }
 return NULL;
}

//-------------------------------- LX_v4lx_mmap --------------------------------
int LX_v4lx_mmap(int handle,void **mmap,int size)
{
 struct video_device *v=LX_v4lx_get_opened_device(handle);
 struct vm_mapped_mem *pMap=NULL;
 unsigned long mhandle;
 int rc=0;
 spin_lock_irq(&mmap_lock);
 mhandle=mmap_handle++;
 if(mmap_handle>255)
  mmap_handle=0;
 spin_unlock_irq(&mmap_lock);
 if(!v)
  return -ENXIO;
 if(!(v->fops->mmap))
  return -ENOSYS;
// rc=v->fops->mmap(v,(char*)mhandle,size); // ToDo: (SM)
 if(rc)
  return rc;
 pMap=LX_get_mapped_mem(mhandle);
 if(pMap)
 {
  (*mmap)=pMap->mAddr;
  return 0;
 }
 return -EINVAL;
}
