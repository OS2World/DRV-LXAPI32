/* $Id: oak_init_task.c,v 1.3 2005/06/04 23:39:36 smilcke Exp $ */

/*
 * lak_init_task.c
 * Autor:               Stefan Milcke
 * Erstellt am:         27.06.2004
 * Letzte Aenderung am: 04.06.2005
 *
*/

#include <lxcommon.h>

#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/init_task.h>
#include <linux/fs.h>

#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/desc.h>

static struct fs_struct init_fs = INIT_FS;
static struct files_struct init_files = INIT_FILES;
static struct signal_struct init_signals = INIT_SIGNALS(init_signals);
static struct sighand_struct init_sighand = INIT_SIGHAND(init_sighand);
struct mm_struct init_mm = INIT_MM(init_mm);

EXPORT_SYMBOL(init_mm);

/*
 * Initial thread structure.
 *
 * We need to make sure that this is THREAD_SIZE aligned due to the
 * way process stacks are handled. This is done by having a special
 * "init_task" linker map entry..
 */
#ifdef TARGET_OS2
union thread_union* _init_thread_union=0;
#else
union thread_union init_thread_union=
   __attribute__((__section__(".data.init_task"))) =
      { INIT_THREAD_INFO(init_task) };
#endif

/*
 * Initial task structure.
 *
 * All other task structs will be allocated on slabs in fork.c
 */
struct task_struct init_task = INIT_TASK(init_task);
#ifdef TARGET_OS2
//#define USE_INIT_TASK_BACKUP
#ifdef USE_INIT_TASK_BACKUP
struct task_struct* init_task_backup=0;
#endif
#endif

EXPORT_SYMBOL(init_task);

/*
 * per-CPU TSS segments. Threads are completely 'soft' on Linux,
 * no more per-task TSS's. The TSS size is kept cacheline-aligned
 * so they are allowed to end up in the .data.cacheline_aligned
 * section. Since TSS's are completely CPU-local, we want them
 * on exact cacheline boundaries, to eliminate cacheline ping-pong.
 */
#ifdef TARGET_OS2
struct tss_struct init_tss[NR_CPUS] __cacheline_aligned;
#else
struct tss_struct init_tss[NR_CPUS] __cacheline_aligned = { [0 ... NR_CPUS-1] = INIT_TSS };
#endif

#ifdef TARGET_OS2
void LX_init_init_task(void)
{
 int i,j;
#ifdef USE_INIT_TASK_BACKUP
 if(!init_task_backup)
 {
  init_task_backup=kmalloc(sizeof(struct task_struct),GFP_KERNEL);
  memcpy(init_task_backup,&init_task,sizeof(struct task_struct));
 }
 else
  memcpy(&init_task,init_task_backup,sizeof(struct task_struct));
#endif
 if(!_init_thread_union)
  _init_thread_union=kmalloc(sizeof(union thread_union),GFP_KERNEL);
 init_task.thread_info=&init_thread_info;
 init_thread_info.task=&init_task;
 init_thread_info.exec_domain=&default_exec_domain;
 init_thread_info.flags=0;
 init_thread_info.cpu=0;
// init_thread_info.preempt_count=1;
 init_thread_info.preempt_count=0;
 init_thread_info.addr_limit=KERNEL_DS;
 init_thread_info.restart_block.fn=do_no_restart_syscall;
 for(i=0;i<NR_CPUS;i++)
 {
  init_tss[i].esp0=sizeof(init_stack)+(long)&init_stack;
  init_tss[i].ss0=__KERNEL_DS;
  init_tss[i].esp1=sizeof(init_tss[0])+(long)&init_tss[0];
  init_tss[i].ss1=__KERNEL_CS;
  init_tss[i].ldt=GDT_ENTRY_LDT;
  init_tss[i].io_bitmap_base=INVALID_IO_BITMAP_OFFSET;
  for(j=0;j<IO_BITMAP_LONGS;j++)
   init_tss[i].io_bitmap[j]=~0;
 }
}
#endif
