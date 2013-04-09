/* $Id: lxmemlock.c,v 1.2 2006/04/27 21:17:51 smilcke Exp $ */

/*
 * lxmemlock.c
 * Autor:               Stefan Milcke
 * Erstellt am:         06.01.2006
 * Letzte Aenderung am: 24.04.2006
 *
*/

#define LXMALLOC_DECLARATIONS
#include <lxcommon.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/list.h>

//#define MAX_LOCK_SIZE (PAGE_SIZE*16)
//#define MAX_LOCK_SIZE   (PAGE_SIZE*8)
#define MAX_LOCK_SIZE   (PAGE_SIZE*15)

//-------------------------------- _LX_lock_mem --------------------------------
static int _LX_lock_mem(ULONG addr,ULONG size
                        ,ULONG flags
                        ,void* lock
                        ,ULONG* numLocks)
{
 int rc=0;
 ULONG pgCount;
 char* pLock=(char*)lock;
 ULONG a=addr;
 if(a&0xfff)
 {
  a&=~0xfff;
  size+=(addr-a);
 }
 (*numLocks)=0;
 if(size&0xfff)
  size+=PAGE_SIZE;
 size&=~0xfff;
 while(size && !rc)
 {
  ULONG sz;
//  if((flags&VMDHL_CONTIGUOUS) && (MAX_LOCK_SIZE<=size))
  if(MAX_LOCK_SIZE<=size)
   sz=MAX_LOCK_SIZE;
  else
   sz=size;
  rc=DevVMLock(flags,a,sz,(LINEAR)-1,(LINEAR)pLock,(LINEAR)&pgCount);
  if(!rc)
  {
   a+=sz;
   size-=sz;
   (*numLocks)++;
   pLock+=12;
  }
 }
 return rc;
}

//------------------------------- _LX_unlock_mem -------------------------------
static void _LX_unlock_mem(void* lock,ULONG numLocks)
{
 char* pLock=(char*)lock;
 while(numLocks)
 {
  DevVMUnLock((LINEAR)pLock);
  pLock+=12;
  numLocks--;
 }
}

struct lxlockstruct
{
 struct list_head list;
 unsigned long addr;
 unsigned long size;
 unsigned long flags;
 atomic_t use_count;
 unsigned long num_locks;
 char *lock;
};

static LIST_HEAD(lxlock_list);
static spinlock_t lx_lock_lock=SPIN_LOCK_UNLOCKED;

//----------------------------- LX_get_lockstruct ------------------------------
static struct lxlockstruct* LX_get_lockstruct(unsigned long addr)
{
 struct list_head* lh;
 struct lxlockstruct* p;
 list_for_each(lh,&lxlock_list)
 {
  p=list_entry(lh,struct lxlockstruct,list);
  if(addr==p->addr)
   return p;
 }
 return 0;
}

//-------------------------------- LX_lock_mem ---------------------------------
int LX_lock_mem(unsigned long addr,unsigned long size,unsigned long flags)
{
 unsigned long f;
 struct lxlockstruct* p;
 char* lock;
 spin_lock_irqsave(&lx_lock_lock,f);
 p=LX_get_lockstruct(addr);
 if(p)
 {
  atomic_inc(&(p->use_count));
  spin_unlock_irqrestore(&lx_lock_lock,f);
  return 0;
 }
 spin_unlock_irqrestore(&lx_lock_lock,f);
 lock=kmalloc(((size/MAX_LOCK_SIZE)+4)*12,GFP_KERNEL);
 p=kmalloc(sizeof(struct lxlockstruct),GFP_KERNEL);
 if(!lock && !p)
 {
  if(lock)
   kfree(lock);
  if(p)
   kfree(p);
  return -ENOMEM;
 }
 p->addr=addr;
 p->size=size;
 p->flags=flags;
 atomic_set(&(p->use_count),1);
 p->num_locks=0;
 p->lock=lock;
 spin_lock_irqsave(&lx_lock_lock,f);
 if(!_LX_lock_mem(addr,size,flags,lock,&p->num_locks))
  list_add(&p->list,&lxlock_list);
 else
 {
  _LX_unlock_mem(lock,p->num_locks);
  kfree(lock);
  kfree(p);
  spin_unlock_irqrestore(&lx_lock_lock,f);
  return -ENOMEM;
 }
 spin_unlock_irqrestore(&lx_lock_lock,f);
 return 0;
}

//------------------------------- LX_unlock_mem --------------------------------
int LX_unlock_mem(unsigned long addr)
{
 unsigned long f;
 int rc;
 struct lxlockstruct* p;
 spin_lock_irqsave(&lx_lock_lock,f);
 p=LX_get_lockstruct(addr);
 if(!p)
  rc=-ENOENT;
 else
 {
  if(atomic_dec_and_test(&p->use_count))
  {
   _LX_unlock_mem(p->lock,p->num_locks);
   list_del(&p->list);
   kfree(p->lock);
   kfree(p);
   rc=0;
  }
  else
   rc=atomic_read(&p->use_count);
 }
 spin_unlock_irqrestore(&lx_lock_lock,f);
 return rc;
}
