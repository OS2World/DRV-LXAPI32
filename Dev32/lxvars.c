/* $Id: lxvars.c,v 1.24 2006/04/27 21:17:53 smilcke Exp $ */

/*
 * lxvars.c
 * Autor:               Stefan Milcke
 * Erstellt am:         03.04.2004
 * Letzte Aenderung am: 25.04.2006
 *
*/

#include <lxcommon.h>
#include <linux/kernel.h>
#include <asm/atomic.h>
#include <asm/page.h>
#include <linux/string.h>

#include <lxdaemon.h>

char lx_drv_homepath[LX_MAXPATH];
char lx_root_path[LX_MAXPATH];
char lx_log_file_name[LX_MAXPATH];
char lx_log_path[LX_MAXPATH];
struct InfoSegGDT* lx_InfoSegGDTPtr=0;
struct InfoSegLDT* lx_InfoSegLDTPtr=0;
unsigned long* lx_IrqLevelPtr=0;
unsigned int lx_num_cpus=1;
unsigned long lx_use_sec_helpers=1;
int lx_is_smp=1;
int lx_do_last_startup_check=1;
int lx_startup_ok=1;
atomic_t lx_kernel_running={0};
atomic_t lx_current_available={0};
atomic_t lx_exit_kernel={0};
atomic_t lx_in_ISR={0};
DECLARE_WAIT_QUEUE_HEAD(lx_shutdown_wq);
unsigned long lx_kernel_ds_size=0;
unsigned long lx_kernel_cs_size=0;
unsigned long lx_kernel_resident_cs_size=0;
unsigned long lx_kernel_resident_ds_size=0;
unsigned long lx_kernel_swappable_cs_size=0;
unsigned long lx_kernel_swappable_ds_size=0;
unsigned short* P_INIT_COUNT=0;
volatile int lx_exit_all_kernel_threads=0;
struct task_struct* lx_idle_task=0;
struct task_struct* lx_init_task=0;

struct lx_sysvars
{
 ULONG max_path_length;
 ULONG max_text_sessions;
 ULONG max_pm_sessions;
 ULONG max_vdm_sessions;
 ULONG boot_drive;
 ULONG dyn_pri_variation;
 ULONG max_wait;
 ULONG min_slice;
 ULONG max_slice;
 ULONG page_size;
 ULONG version_major;
 ULONG version_minor;
 ULONG version_revision;
 ULONG ms_count;
 ULONG time_low;
 ULONG time_high;
 ULONG totphysmem;
 ULONG totresmem;
 ULONG totavailmem;
 ULONG maxprmem;
 ULONG maxshmem;
 ULONG timer_interval;
 ULONG max_comp_length;
 ULONG foreground_fs_session;
 ULONG foreground_process;
};

struct lx_sysvars lx_sysvars={0};
unsigned long lx_totavail_pages=0;
unsigned long totalram_pages=0;
unsigned long max_pfn=0;
extern unsigned long num_physpages;
struct PDDStack *lx_StackRoot=0;
struct PDDStack *lx_IrqStackRoot=0;
struct UTLStack *lx_UtlStackRoot=0;
#ifdef LXDEBUG
volatile unsigned long lx_call_depth=0;
#endif
unsigned long lx_sysstate=LXSYSSTATE_NOT_INITIALIZED | LXSYSSTATE_NOT_OPERABLE;
unsigned long lx_default_malloc_flags=VMDHA_USEHIGHMEM | VMDHA_FIXED;
unsigned long lx_memobj_malloc_flags=0;

//#define LX_MEM_DIVISOR  (100)
#define LX_MEM_DIVISOR  (50)
//#define LX_MEM_DIVISOR  (25)
//#define LX_MEM_DIVISOR  (20)
//#define LX_MEM_DIVISOR  (15)
//#define LX_MEM_DIVISOR  (10)   // Release version!!!
//#define LX_MEM_DIVISOR  (5)

unsigned long lx_mem_divisor=LX_MEM_DIVISOR;

//------------------------------ LX_init_sysvars -------------------------------
void LX_init_sysvars(void)
{
 if(!LX_DOSQUERYSYSINFO(1,25,&lx_sysvars,sizeof(lx_sysvars)))
 {
  num_physpages=(lx_sysvars.totphysmem/lx_mem_divisor)/PAGE_SIZE;
  totalram_pages=(lx_sysvars.totphysmem/lx_mem_divisor)/PAGE_SIZE;
  max_pfn=((lx_sysvars.totavailmem/lx_mem_divisor)/1024/1024) << (20 - PAGE_SHIFT);

  lx_totavail_pages=((lx_sysvars.totphysmem/lx_mem_divisor)-lx_kernel_ds_size-lx_kernel_cs_size)
                    /PAGE_SIZE;
 }
}

