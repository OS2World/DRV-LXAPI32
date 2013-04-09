/* $Id: lxwaiton.c,v 1.3 2005/03/27 20:59:31 smilcke Exp $ */

/*
 * lxwaiton.c
 * Autor:               Stefan Milcke
 * Erstellt am:         16.11.2004
 * Letzte Aenderung am: 27.03.2005
 *
*/

#include <lxcommon.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/errno.h>

#define LXWF_ISWQ             1
#define LXWF_WAITCONTINUE     2
struct lx_waitid
{
 struct list_head list;
 unsigned long id;
 long timeout;
 unsigned long blckflags;
 unsigned long flags;
 wait_queue_head_t wq;
};

static LIST_HEAD(lx_wait_list);
static spinlock_t lx_wait_list_lock=SPIN_LOCK_UNLOCKED;

//--------------------------------- LX_WaitOn ----------------------------------
unsigned long LX_WaitOn(unsigned long id,unsigned long timeout,unsigned long flags)
{
 unsigned long rc=0;
 unsigned long prev_current_available=atomic_read(&lx_current_available);
 struct lx_waitid wi=
 {
  .id=id,
  .timeout=timeout,
  .blckflags=flags,
  .flags=0,
 };
 if(atomic_read(&lx_current_available))
 {
  wi.flags=LXWF_ISWQ;
  LX_enter_current();
 }
 init_waitqueue_head(&(wi.wq));
 spin_lock(&lx_wait_list_lock);
 list_add(&(wi.list),&lx_wait_list);
 spin_unlock(&lx_wait_list_lock);
 do
 {
  wi.flags&=~LXWF_WAITCONTINUE;
  if(wi.flags&LXWF_ISWQ)
  {
   set_current_state(TASK_INTERRUPTIBLE);
   if(wi.timeout>=0)
   {
    if(sleep_on_timeout(&(wi.wq),timeout))
     rc=0;
    else
     rc=1;
   }
   else
    sleep_on(&(wi.wq));
  }
  else
  {
   rc=DevBlock(wi.id,wi.timeout,wi.blckflags);
   if(atomic_read(&lx_current_available))
   {
    if(!prev_current_available)
    {
     wi.flags|=LXWF_ISWQ;
     LX_enter_current();
     prev_current_available=1;
    }
   }
  }
 }while(wi.flags&LXWF_WAITCONTINUE);
 spin_lock(&lx_wait_list_lock);
 list_del(&(wi.list));
 spin_unlock(&lx_wait_list_lock);
 return rc;
}

//---------------------------------- LX_RunOn ----------------------------------
unsigned long LX_RunOn(unsigned long id)
{
 unsigned long rc=0;
 struct list_head* lh;
 spin_lock(&lx_wait_list_lock);
 list_for_each(lh,&lx_wait_list)
 {
  struct lx_waitid* p=list_entry(lh,struct lx_waitid,list);
  if(id==p->id)
  {
   if(p->flags&LXWF_ISWQ)
    wake_up_all(&p->wq);
   else
    rc=DevRun(p->id);
  }
 }
 spin_unlock(&lx_wait_list_lock);
 return rc;
}

//------------------------------ LX_ReschedWaitOn ------------------------------
void LX_ReschedWaitOn(void)
{
 struct list_head* lh;
 struct lx_waitid* p;
 int do_resched=0;
 if(atomic_read(&lx_current_available))
 {
  spin_lock(&lx_wait_list_lock);
  list_for_each(lh,&lx_wait_list)
  {
   p=list_entry(lh,struct lx_waitid,list);
   if(!(p->flags&LXWF_ISWQ))
   {
    p->flags|=LXWF_WAITCONTINUE;
    DevRun(p->id);
    do_resched++;
   }
  }
  spin_unlock(&lx_wait_list_lock);
 }
 if(do_resched)
 {
  current->state=TASK_INTERRUPTIBLE;
  schedule_timeout(100);
  current->state=TASK_RUNNING;
  LX_ReschedWaitOn();
 }
}
