/* $Id: oo_thread.c,v 1.11 2005/05/02 23:56:09 smilcke Exp $ */

/*
 * oo_thread.c
 * Autor:               Stefan Milcke
 * Erstellt am:         11.10.2004
 * Letzte Aenderung am: 03.05.2005
 *
*/

#include <lxcommon.h>

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/err.h>
#include <asm/hardirq.h>

#include <lxapi.h>
#include <lxdaemon.h>

static spinlock_t lx_task_start_lock=SPIN_LOCK_UNLOCKED;
struct NTASK_struct
{
 unsigned long esp;
 unsigned long eip;
 unsigned long prev;
};

struct lx_new_task_struct
{
 struct list_head list;
 unsigned long esp;
 unsigned long eip;
 unsigned long prev;
 unsigned long flags;
 unsigned long tid;
};

static LIST_HEAD(lx_task_to_start_list);
static spinlock_t lx_task_to_start_lock=SPIN_LOCK_UNLOCKED;

extern struct NTASK_struct lx_started_task;

extern void __LX_wakeup_softirq(void);
extern void fastcall LXA_TaskStub32(void);
extern asmlinkage void do_softirq(void);
extern void LX_check_utl_freeme(void);

static atomic_t lx_softirq_wakes={0};

extern volatile unsigned long lxa_service_thread_block_time;
extern volatile unsigned long lxa_service_thread_id;
//------------------------------ LX_ServiceThread ------------------------------
void LX_ServiceThread(void)
{
 unsigned long f;
 // Do any outstanding SOFTIRQs
 if(atomic_read(&lx_softirq_wakes)!=0)
 {
  LX_enter_irq_current();
  atomic_set(&lx_softirq_wakes,0);
  do_softirq();
  LX_leave_irq_current();
  LX_check_utl_freeme();
 }
 // Check if we have to start a new thread
 if(lx_started_task.eip || lx_started_task.esp || lx_started_task.prev)
  lxa_service_thread_block_time=10;
 else if(!list_empty(&lx_task_to_start_list))
 {
  struct lx_new_task_struct* p=0;
  spin_lock_irqsave(&lx_task_to_start_lock,f);
  if(!list_empty(&lx_task_to_start_list))
  {
   p=list_entry(lx_task_to_start_list.next,struct lx_new_task_struct,list);
   list_del(&p->list);
   lx_started_task.prev=p->prev;
   lx_started_task.esp=p->esp;
   lx_started_task.eip=p->eip;  // This must be the last one!
  }
  spin_unlock_irqrestore(&lx_task_to_start_lock,f);
  if(p)
  {
   LXA_CreateThread((unsigned long)LXA_TaskStub32,&p->tid);
   if(!(p->flags&LX_STARTTASK_WAIT))
    kfree(p);
   else
    p->flags=LX_STARTTASK_STARTED;
   DevRun((unsigned long)&lx_task_to_start_list);
  }
  lxa_service_thread_block_time=10;
 }
 else
  lxa_service_thread_block_time=10000;
}

//----------------------------- LX_wakeup_softirq ------------------------------
void LX_wakeup_softirq(void)
{
 atomic_inc(&lx_softirq_wakes);
 DevRun((unsigned long)&lxa_service_thread_id);
}

//------------------------------ LX_StartNewTask -------------------------------
unsigned long LX_StartNewTask(unsigned long esp,unsigned long eip
                              ,unsigned long prev
                              ,unsigned long flags)
{
 unsigned long rc=0;
 unsigned long f;
 struct lx_new_task_struct* p=kmalloc(sizeof(struct lx_new_task_struct),GFP_KERNEL);
 if(!p)
  return -ENOMEM;
 p->esp=esp;
 p->eip=eip;
 p->prev=prev;
 if(in_interrupt())
  flags&=~LX_STARTTASK_WAIT;
 p->flags=flags;
 spin_lock_irqsave(&lx_task_to_start_lock,f);
 list_add_tail(&p->list,&lx_task_to_start_list);
 spin_unlock_irqrestore(&lx_task_to_start_lock,f);
 DevRun((unsigned long)&lxa_service_thread_id);
 if(flags&LX_STARTTASK_WAIT)
 {
  while(!(p->flags&LX_STARTTASK_STARTED))
  {
   DevRun((unsigned long)&lxa_service_thread_id);
   DevBlock((unsigned long)&lx_task_to_start_list,10,0);
  }
  rc=p->tid;
  kfree(p);
 }
 return rc;
}

