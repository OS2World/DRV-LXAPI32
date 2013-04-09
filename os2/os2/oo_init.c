/* $Id: oo_init.c,v 1.8 2006/02/16 23:07:21 smilcke Exp $ */

/*
 * oo_init.c
 * Autor:               Stefan Milcke
 * Erstellt am:         13.11.2004
 * Letzte Aenderung am: 12.01.2006
 *
*/

#include <lxcommon.h>
#include <linux/kernel.h>
#include <linux/string.h>

#include <lxapi.h>
#include <lxdaemon.h>

extern void LX_init_core(void);
extern int LX_check_last_startup(void);
extern void LX_init_sysvars(void);

//------------------------------- LX_init_thread -------------------------------
int LX_init_thread(void* dummy)
{
 struct task_struct* p=current;
 while(*P_INIT_COUNT<255 && 0!=*P_INIT_COUNT)
  DevBlock((unsigned long)P_INIT_COUNT,10,0);
 cpu_set(0,cpu_callout_map);
 printk("Querying system info...");
 LX_init_sysvars();
 printk(" O.K.\n");
 atomic_set(&lx_exit_kernel,0);
 if(LX_check_last_startup())
  LX_set_continue_startup();
 else
 {
  if(lx_is_smp)
  {
   int i;
   for(i=0;i<lx_num_cpus;i++)
    cpu_set(i,cpu_callout_map);
   printk("SMP detected with %d CPUs\n",lx_num_cpus);
  }
  else
   printk("No SMP detected\n");
  LX_init_core();
  LX_process_script("boot\\boot");
  atomic_set(&lx_kernel_running,1);
  if(!lx_daemon_pid)
  {
   printk("Daemon not started!\n");
   LX_set_continue_startup();
  }
 }
// LX_leave_current();
 return 0;
}

//---------------------------- LX_start_init_thread ----------------------------
void LX_start_init_thread(void)
{
 struct task_struct* tsk;
 struct thread_info* ti;
 tsk=(struct task_struct*)kmalloc(sizeof(struct task_struct),GFP_KERNEL);
 ti=(struct thread_info*)kmalloc(sizeof(union thread_union),GFP_KERNEL);
 if(!tsk || !ti)
 {
  if(tsk)
   kfree(tsk);
  if(ti)
   kfree(ti);
  return;
 }
 memcpy(tsk,&init_task,sizeof(struct task_struct));
 memcpy(ti,init_task.thread_info,sizeof(struct thread_info));
 tsk->thread_info=ti;
 ti->task=tsk;
 LX_StartNewTask(((unsigned long)ti)+(THREAD_SIZE)
                 ,(unsigned long)&LX_init_thread
                 ,0
                 ,LX_STARTTASK_WAIT);
}

asmlinkage void startup_32(void) __asm__("startup_32");
extern void LX_enter_init_current(void);
//----------------------------- LX_kernel_starter ------------------------------
void LX_kernel_starter(void)
{
 LX_StartNewTask(((unsigned long)(init_task.thread_info))+(THREAD_SIZE)
                 ,(unsigned long)&startup_32
                 ,0
                 ,LX_STARTTASK_WAIT);
 while(*P_INIT_COUNT!=0 && !atomic_read(&lx_current_available))
  DevBlock((unsigned long)P_INIT_COUNT,10,0);
 while(!lx_init_task)
  DevBlock((unsigned long)current,10,0);
 LX_enter_init_current();
 while(*P_INIT_COUNT!=0)
 {
  set_current_state(TASK_INTERRUPTIBLE);
  schedule();
  set_current_state(TASK_RUNNING);
 }
 LX_leave_current();
}
