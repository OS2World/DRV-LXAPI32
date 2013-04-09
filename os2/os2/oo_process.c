/* $Id: oo_process.c,v 1.14 2005/04/18 22:28:44 smilcke Exp $ */

/*
 * lx_process.c
 * Autor:               Stefan Milcke
 * Erstellt am:         21.02.2003
 * Letzte Aenderung am: 11.04.2005
 *
*/

#include <lxcommon.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <asm/semaphore.h>
#include <linux/init.h>
#include <linux/init_task.h>
#include <asm/hardirq.h>
#include <linux/security.h>
#include <asm/uaccess.h>
#include <linux/smp_lock.h>

#include <lxapi.h>
#include <lxapioctl.h>

#define LXC_FLAG_INITIALIZED           1
#define LXC_FLAG_TSK_MALLOCED          2
#define LXC_FLAG_TSK_REGULAR_EXIT      4

struct lx_current
{
 struct list_head list;
 unsigned short os2_pid;
 unsigned short os2_tid;
 unsigned long current_flags;
 struct thread_info* thread_info;
 struct task_struct* task;
};

static LIST_HEAD(lx_current_list);
static LIST_HEAD(lx_exit_list);
static spinlock_t lx_current_lock=SPIN_LOCK_UNLOCKED;
static struct task_struct* lx_current_task=0;
struct thread_info* lx_current_thread_info=0;

extern struct task_struct *LX_copy_process(struct task_struct* from,
             struct task_struct* p,
             unsigned long clone_flags,
             unsigned long stack_start,
             struct pt_regs *regs,
             unsigned long stack_size,
             int __user *parent_tidptr,
             int __user *child_tidptr);
extern unsigned long total_forks;
extern asmlinkage void schedule_tail(task_t* prev);

//-------------------------------- LX_init_task --------------------------------
void LX_init_task(struct task_struct* tsk_from
                  ,struct task_struct* tsk
                  ,struct thread_info* ti_from
                  ,struct thread_info* ti)
{
 struct pt_regs regs;
 struct pt_regs* poregs;
 struct pt_regs oregs;
 unsigned long clone_flags;
 memset(&regs,0,sizeof(regs));
 regs.xds = __USER_DS;
 regs.xes = __USER_DS;
 regs.orig_eax = -1;
 regs.xcs = __KERNEL_CS;
 regs.eflags = X86_EFLAGS_IF | X86_EFLAGS_SF | X86_EFLAGS_PF | 0x2;
 clone_flags=  CLONE_VM
             | CLONE_UNTRACED
             | CLONE_FS
             | CLONE_SIGHAND
             | CLONE_KERNEL
             | CLONE_FILES
             | SIGCHLD;
 if(tsk_from!=tsk)
  memcpy(tsk,tsk_from,sizeof(struct task_struct));
 if(ti_from!=ti)
  memcpy(ti,ti_from,sizeof(struct thread_info));
 tsk->thread_info=ti;
 ti->task=tsk;
 poregs=((struct pt_regs*)(THREAD_SIZE+(unsigned long)tsk->thread_info))-1;
 memcpy(&oregs,poregs,sizeof(struct pt_regs));
 { // Similary to do_fork()
  if(LX_copy_process(tsk_from,tsk
                     ,clone_flags
                     ,0
                     ,&regs
                     ,0
                     ,NULL
                     ,NULL))
  {
   struct task_struct* p=tsk;
   long pid=tsk->pid;
   tsk->exit_signal=-1;
   if(!(clone_flags&CLONE_STOPPED))
    wake_up_forked_process(tsk);
   else
    tsk->state=TASK_STOPPED;
   ++total_forks;
   set_need_resched();
   schedule_tail(tsk_from);
//   schedule();
  }
  else
   DevInt3();
 }
 memcpy(poregs,&oregs,sizeof(struct pt_regs));
}

extern void LX_exit_current_exit(struct task_struct* tsk);
//-------------------------------- LX_exit_task --------------------------------
void LX_exit_task(struct task_struct* tsk)
{
 LX_exit_current_exit(tsk);
}

//-------------------------- LX_get_lxcurrent_from_ti --------------------------
static struct lx_current* LX_get_lxcurrent_from_ti(struct thread_info* ti)
{
 struct list_head* lh;
 struct lx_current* p=0;
 list_for_each(lh,&lx_current_list)
 {
  p=list_entry(lh,struct lx_current,list);
  if(p->thread_info==ti)
   return p;
 }
 return 0;
}

//------------------------- LX_get_lxcurrent_from_tsk --------------------------
static struct lx_current* LX_get_lxcurrent_from_tsk(struct task_struct* tsk)
{
 struct list_head* lh;
 struct lx_current* p=0;
 list_for_each(lh,&lx_current_list)
 {
  p=list_entry(lh,struct lx_current,list);
  if(p->task==tsk)
   return p;
 }
 return 0;
}

//------------------------ LX_get_lxcurrent_from_pidtid ------------------------
static struct lx_current* LX_get_lxcurrent_from_pidtid(unsigned short pid
                                                       ,unsigned short tid)
{
 struct list_head* lh;
 struct lx_current* p=0;
 list_for_each(lh,&lx_current_list)
 {
  p=list_entry(lh,struct lx_current,list);
  if(pid==p->os2_pid && tid==p->os2_tid)
   return p;
 }
 return 0;
}

//---------------------------- LX_add_this_current -----------------------------
void LX_add_this_current(void)
{
 unsigned long f;
 struct lx_current* p=0;
 struct thread_info* ti=current_thread_info();
 if(!ti->task)
 {
  struct task_struct *from;
  if(lx_init_task)
   from=lx_init_task;
  else
   from=&init_task;
  memcpy(ti,from->thread_info,sizeof(struct thread_info));
  ti->task=from;
 }
 spin_lock_irqsave(&lx_current_lock,f);
 p=LX_get_lxcurrent_from_ti(ti);
 spin_unlock_irqrestore(&lx_current_lock,f);
 if(!p)
 {
  p=kmalloc(sizeof(struct lx_current),GFP_KERNEL);
  p->thread_info=ti;
  p->task=ti->task;
  p->task->thread_info=ti;
  p->os2_pid=lx_current_pid;
  p->os2_tid=lx_current_tid;
  p->current_flags=LXC_FLAG_INITIALIZED;
  spin_lock_irqsave(&lx_current_lock,f);
  list_add(&p->list,&lx_current_list);
  spin_unlock_irqrestore(&lx_current_lock,f);
  if(1==ti->task->pid)
  {
   atomic_set(&lx_current_available,1);
  }
 }
}

//------------------------------ LX_enter_current ------------------------------
void LX_enter_current(void)
{
 unsigned long f;
 int is_new=0;
 struct lx_current* p=0;
 struct thread_info* ti=current_thread_info();
 spin_lock_irqsave(&lx_current_lock,f);
 p=LX_get_lxcurrent_from_ti(ti);
 spin_unlock_irqrestore(&lx_current_lock,f);
 if(!p || !ti->task)
 {
  ti->exec_domain=&default_exec_domain;
  ti->flags=0;
  if(ti->cpu)
   ti->cpu=0;
  ti->preempt_count=0;
// ti->preempt_count=1;
  ti->addr_limit=KERNEL_DS;
  ti->restart_block.fn=do_no_restart_syscall;
 }
 if(!p)
 {
  p=kmalloc(sizeof(struct lx_current),GFP_KERNEL);
  p->thread_info=ti;
  p->task=kmalloc(sizeof(struct task_struct),GFP_KERNEL);
  ti->task=p->task;
  p->task->thread_info=ti;
  p->os2_pid=lx_current_pid;
  p->os2_tid=lx_current_tid;
  p->current_flags=LXC_FLAG_TSK_MALLOCED;
  is_new=1;
 }
 if(p && p->task)
 {
  if(0!=lx_init_task && !(p->current_flags&LXC_FLAG_INITIALIZED))
  {
   LX_init_task(lx_init_task,p->task,ti,ti);
   p->current_flags|=LXC_FLAG_INITIALIZED;
  }
  else
  {
   ti->task=p->task;
   p->task->thread_info=ti;
  }
 }
 if(p && 0!=is_new)
 {
  spin_lock_irqsave(&lx_current_lock,f);
  list_add(&p->list,&lx_current_list);
  spin_unlock_irqrestore(&lx_current_lock,f);
 }
}

//--------------------------- LX_enter_init_current ----------------------------
void LX_enter_init_current(void)
{
 unsigned long f;
 struct lx_current* p=0;
 struct thread_info* ti=current_thread_info();
 if(LX_is_utl_current(ti))
  return;
 spin_lock_irqsave(&lx_current_lock,f);
 p=LX_get_lxcurrent_from_ti(ti);
 if(p)
  p->current_flags&=~LXC_FLAG_INITIALIZED;
 spin_unlock_irqrestore(&lx_current_lock,f);
 LX_enter_current();
}

//------------------------------ LX_leave_current ------------------------------
void LX_leave_current(void)
{
 unsigned long f;
 struct lx_current* p=0;
 struct thread_info* ti=current_thread_info();
 if(LX_is_utl_current(ti))
  return;
 spin_lock_irqsave(&lx_current_lock,f);
 p=LX_get_lxcurrent_from_ti(ti);
 if(p && p->task)
 {
  list_del(&p->list);
  spin_unlock_irqrestore(&lx_current_lock,f);
  if(atomic_read(&lx_current_available))
  {
   if(p->current_flags&LXC_FLAG_INITIALIZED)
   {
    p->current_flags&=~LXC_FLAG_INITIALIZED;
    if(p->task==lx_current_task)
    {
     spin_lock_irqsave(&lx_current_lock,f);
     lx_current_task=0;
     lx_current_thread_info=0;
     spin_unlock_irqrestore(&lx_current_lock,f);
    }
    LX_exit_task(p->task);
   }
  }
  if(p->current_flags&LXC_FLAG_TSK_MALLOCED)
  {
   kfree(p->task);
   p->task=0;
   ti->task=0;
  }
  kfree(p);
  memset(ti,0,sizeof(struct thread_info));
 }
 else
  spin_unlock_irqrestore(&lx_current_lock,f);
}

//--------------------------- _LX_enter_irq_current ----------------------------
void LX_enter_irq_current(void)
{
 struct thread_info* cti=current_thread_info();
 struct thread_info* ti;
 if(lx_current_thread_info)
  ti=lx_current_thread_info;
 else
  ti=&init_thread_info;
 if(ti->cpu>NR_CPUS)
 {
  DevInt3();
  ti->cpu=0;
 }
 if(cti!=ti)
 {
  memcpy(cti,ti,sizeof(struct thread_info));
  cti->task->thread_info=cti;
 }
}

//---------------------------- LX_leave_irq_current ----------------------------
void LX_leave_irq_current(void)
{
 struct thread_info* cti=current_thread_info();
 struct thread_info* ti;
 if(lx_current_thread_info)
  ti=lx_current_thread_info;
 else
  ti=&init_thread_info;
 if(ti->cpu>NR_CPUS)
 {
  DevInt3();
  ti->cpu=0;
 }
 if(cti!=ti)
 {
  memcpy(ti,cti,sizeof(struct thread_info));
  if(ti==&init_thread_info)
  {
   init_task.thread_info=&init_thread_info;
   init_thread_info.task=&init_task;
  }
  else
  {
   lx_current_thread_info->task=lx_current_task;
   lx_current_task->thread_info=lx_current_thread_info;
  }
 }
}

static spinlock_t lx_utlstack_lock=SPIN_LOCK_UNLOCKED;

//---------------------------- LX_check_utl_freeme -----------------------------
void LX_check_utl_freeme(void)
{
 unsigned long f;
 struct UTLStack* prev=0;
 struct UTLStack* p;
 spin_lock_irqsave(&lx_utlstack_lock,f);
 p=lx_UtlStackRoot;
 while(p)
 {
  if(p->flags&UTLSTACK_FLAG_FREEME)
  {
   if(!(p->flags&UTLSTACK_FLAG_USED))
   {
    p->flags|=UTLSTACK_FLAG_USED;
    if(prev)
     prev->next=p->next;
    else
     lx_UtlStackRoot=p->next;
    spin_unlock_irqrestore(&lx_utlstack_lock,f);
    kfree((void*)p->base);
    kfree(p);
    spin_lock_irqsave(&lx_utlstack_lock,f);
    p=lx_UtlStackRoot;
    prev=0;
    continue;
   }
  }
  prev=p;
  p=p->next;
 }
 spin_unlock_irqrestore(&lx_utlstack_lock,f);
}

//-------------------------- LX_find_utlstack_pidtid ---------------------------
static struct UTLStack* LX_find_utlstack_pidtid(unsigned long pid,unsigned long tid)
{
 struct UTLStack* p=lx_UtlStackRoot;
 while(p)
 {
  if(p->pid==pid && p->tid==tid)
   return p;
  p=p->next;
 }
 return 0;
}

//---------------------------- LX_find_utlstack_ti -----------------------------
static struct UTLStack* LX_find_utlstack_ti(struct thread_info* ti)
{
 struct UTLStack* p=lx_UtlStackRoot;
 while(p)
 {
  if(p->base==(unsigned long)ti)
   return p;
  p=p->next;
 }
 return 0;
}

//---------------------------- LX_set_ti_for_pidtid ----------------------------
int LX_set_ti_for_pidtid(struct thread_info* ti,unsigned short pid,unsigned short tid)
{
 unsigned long f;
 struct UTLStack* p;
 struct UTLStack* pp=kmalloc(sizeof(struct UTLStack),GFP_KERNEL);
 if(!pp)
  return -ENOMEM;
 spin_lock_irqsave(&lx_utlstack_lock,f);
 p=LX_find_utlstack_pidtid(pid,tid);
 if(p)
 {
  spin_unlock_irqrestore(&lx_utlstack_lock,f);
  kfree(pp);
  return -EBUSY;
 }
 pp->base=(unsigned long)ti;
 pp->pid=pid;
 pp->tid=tid;
 pp->flags=0;
 pp->next=lx_UtlStackRoot ? lx_UtlStackRoot->next : 0;
 lx_UtlStackRoot=pp;
 spin_unlock_irqrestore(&lx_utlstack_lock,f);
 return 0;
}

//--------------------------- LX_unset_ti_for_pidtid ---------------------------
int LX_unset_ti_for_pidtid(struct thread_info* ti,unsigned short pid,unsigned short tid)
{
 unsigned long f;
 struct UTLStack* p;
 struct UTLStack* pp=0;
 if(tid)
  tid--;
 spin_lock_irqsave(&lx_utlstack_lock,f);
 p=lx_UtlStackRoot;
 while(p)
 {
  if(p->pid==pid && p->tid==tid)
  {
   if(pp)
    pp->next=p->next;
   else
    lx_UtlStackRoot=p->next;
   kfree(p);
   break;
  }
  p=p->next;
 }
 spin_unlock_irqrestore(&lx_utlstack_lock,f);
 return 0;
}

extern void fastcall
  LXA_UtilStackWaitForCaptureAndExit(struct UTLStack* p,unsigned long no_exit_thread);
//--------------------- LX_UtilStackWaitForCaptureAndExit ----------------------
void LX_UtilStackWaitForCaptureAndExit(unsigned short pid,unsigned short tid
                                       ,unsigned long no_exit_thread)
{
 struct UTLStack* p=LX_find_utlstack_pidtid(pid,tid);
 if(p)
  LXA_UtilStackWaitForCaptureAndExit(p,no_exit_thread);
}

//---------------------------- LX_enter_utl_current ----------------------------
void LX_enter_utl_current(void)
{
 struct UTLStack* p=LX_find_utlstack_ti(current_thread_info());
 if(!p)
  LX_enter_current();
 else
 {
  if(!(p->flags&UTLSTACK_FLAG_CAPTURED))
  {
   int c=100;
   p->flags|=UTLSTACK_FLAG_CAPTURED;
   while(!(p->flags&UTLSTACK_FLAG_CAPTUREACK) && c>0)
   {
    DevBlock((unsigned long)p,100,0);
    c--;
   }
  }
 }
}

//---------------------------- LX_leave_utl_current ----------------------------
void LX_leave_utl_current(int flags)
{
 struct UTLStack* p=LX_find_utlstack_ti(current_thread_info());
 if(!p)
  LX_leave_current();
 else
 {
  if(flags&LXLEAVE_UTL_DOEXIT)
  {
  }
  if(flags&LXLEAVE_UTL_FREEME)
  {
   p->flags|=UTLSTACK_FLAG_FREEME;
  }
 }
}

//----------------------------- LX_is_utl_current ------------------------------
int LX_is_utl_current(struct thread_info* ti)
{
 return (int)(LX_find_utlstack_ti(ti) ? 1 : 0);
}

//------------------------------- LX_is_new_task -------------------------------
static int LX_is_new_task(struct task_struct* prev,struct task_struct* p)
{
 if(p==prev || p==current || p==&init_task)
  return 0;
 if(LX_get_lxcurrent_from_tsk(p))
  return 0;
 if(LX_is_utl_current(p->thread_info))
  return 0;
 return 1;
}

//------------------------- LX_wake_up_forked_process --------------------------
void fastcall LX_wake_up_forked_process(struct task_struct* p)
{
 if(LX_is_new_task(current,p))
 {
  unsigned long eip;
  unsigned long esp;
  unsigned long pr;
  esp=p->thread.esp;
  eip=p->thread.eip;
  pr=(unsigned long)current;
  LX_StartNewTask(esp,eip,pr,LX_STARTTASK_WAIT);
 }
}

//---------------------------------- LX_Block ----------------------------------
#ifndef LXDEBUG
static __inline__
#endif
char LX_Block(struct task_struct* tsk,unsigned long timeout)
{
 return DevBlock((unsigned long)tsk,timeout,0);
}

//----------------------------------- LX_Run -----------------------------------
#ifndef LXDEBUG
static __inline__
#endif
unsigned long LX_Run(struct task_struct* tsk)
{
 return DevRun((unsigned long)tsk);
}

//---------------------------- LX_set_current_task -----------------------------
void LX_set_current_task(struct task_struct* tsk)
{
 unsigned long f;
 spin_lock_irqsave(&lx_current_lock,f);
 if(tsk && tsk->thread_info)
 {
  lx_current_task=tsk;
  lx_current_thread_info=tsk->thread_info;
 }
 else
 {
  lx_current_task=lx_idle_task;
  if(lx_current_task)
   lx_current_thread_info=lx_current_task->thread_info;
  else
   lx_current_thread_info=0;
 }
 spin_unlock_irqrestore(&lx_current_lock,f);
}

//-------------------------- LX_set_task_regular_exit --------------------------
void LX_set_task_regular_exit(struct task_struct* tsk)
{
 unsigned long f;
 struct lx_current* p;
 spin_lock_irqsave(&lx_current_lock,f);
 p=LX_get_lxcurrent_from_tsk(tsk);
 if(p)
  p->current_flags|=LXC_FLAG_TSK_REGULAR_EXIT;
 spin_unlock_irqrestore(&lx_current_lock,f);
}

//------------------------ LX_free_unused_task_structs -------------------------
static void LX_free_unused_task_structs(void)
{
 unsigned long f;
 struct lx_current* p=0;
 struct list_head* lh;
 if(!list_empty(&lx_exit_list))
 {
  spin_lock_irqsave(&lx_current_lock,f);
  p=list_entry(lx_exit_list.next,struct lx_current,list);
  list_del(&p->list);
  spin_unlock_irqrestore(&lx_current_lock,f);
  kfree(p->thread_info);
  kfree(p->task);
  kfree(p);
 }
}

extern void fastcall LXA_DoTaskExit(void);

//------------------------------- LX_DoTaskExit --------------------------------
void LX_DoTaskExit(void)
{
 unsigned long f;
 struct lx_current* p=0;
 spin_lock_irqsave(&lx_current_lock,f);
 p=LX_get_lxcurrent_from_tsk(current);
 if(p)
 {
  list_move(&p->list,&lx_exit_list);
 }
 spin_unlock_irqrestore(&lx_current_lock,f);
 LXA_DoTaskExit();
}

//--------------------------- LX_process_start_error ---------------------------
void fastcall LX_process_start_error(unsigned long pid)
{
 LX_enter_current();
 printk(KERN_WARNING "Wait for PID=%d failed\n",pid);
#if 1
 LX_set_continue_startup();
#else
 *P_INIT_COUNT=0;
#endif
 LX_leave_current();
}

//-------------------------------- LX_switch_to --------------------------------
struct task_struct* LX_switch_to(struct task_struct* prev,struct task_struct* next)
{
 struct task_struct* rc;
 struct task_struct* pc=current;
 struct lx_current* p=0;
 unsigned long f;
 unsigned long c;
 unsigned long ex=(pc->state==TASK_DEAD) || (pc->state==TASK_ZOMBIE);
 unsigned long do_exit_me=0;
 LX_set_current_task(pc);
 rc=__switch_to(prev,next);
 if(prev!=next)
 {
  c=LX_Run(next);
  if(!atomic_read(&lx_in_ISR) && !ex)
   LX_Block(prev,-1);
 }
 LX_free_unused_task_structs();
 spin_lock_irqsave(&lx_current_lock,f);
 if(!ex)
 {
  lx_current_task=pc;
  lx_current_thread_info=lx_current_task->thread_info;
 }
 else
 {
  if(prev!=next)
  {
   lx_current_task=next;
   lx_current_thread_info=lx_current_task->thread_info;
  }
  else
  {
   if(lx_init_task)
    lx_current_task=lx_init_task;
   else if(lx_idle_task)
    lx_current_task=lx_idle_task;
   else
    lx_current_task=0;
   if(lx_current_task)
    lx_current_thread_info=lx_current_task->thread_info;
   else
    lx_current_thread_info=0;
  }
  p=LX_get_lxcurrent_from_tsk(pc);
  if(p && (p->current_flags&LXC_FLAG_TSK_REGULAR_EXIT))
   do_exit_me=1;
 }
 spin_unlock_irqrestore(&lx_current_lock,f);
 if(do_exit_me)
  LX_DoTaskExit();
 return rc;
}
