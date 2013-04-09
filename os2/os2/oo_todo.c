/* $Id: oo_todo.c,v 1.5 2005/02/12 23:58:42 smilcke Exp $ */

/*
 * lx_todo.c
 * Autor:               Stefan Milcke
 * Erstellt am:         04.08.2004
 * Letzte Aenderung am: 12.02.2005
 *
*/

#include <lxcommon.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/cpumask.h>
#include <linux/config.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/blkdev.h>
#include <linux/pagemap.h>
#include <linux/pagevec.h>
#include <asm/processor.h>
#include <asm/i387.h>
#include <asm/uaccess.h>
#include <asm/desc.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/radix-tree.h>

struct zone *zone_table[100];

//--------------------------------- lock_page ----------------------------------
void lock_page(struct page* page)
{
 NOT_IMPLEMENTED();
}

//------------------------------ install_file_pte ------------------------------
int install_file_pte(struct mm_struct *mm, struct vm_area_struct *vma, unsigned long addr, unsigned long pgoff, pgprot_t prot)
{
 NOT_IMPLEMENTED();
 return 0;
}

//------------------------------ shmem_zero_setup ------------------------------
int shmem_zero_setup(struct vm_area_struct *p)
{
 NOT_IMPLEMENTED();
 return 0;
}

//------------------------------- __pagevec_free -------------------------------
void __pagevec_free(struct pagevec *pvec)
{
 NOT_IMPLEMENTED();
}

//------------------------------- free_hot_page --------------------------------
void asmlinkage free_hot_page(struct page* page)
{
 NOT_IMPLEMENTED();
}

//-------------------------------- install_page --------------------------------
int install_page(struct mm_struct *mm, struct vm_area_struct *vma, unsigned long addr, struct page *page, pgprot_t prot)
{
 NOT_IMPLEMENTED();
 return 0;
}

//---------------------------- sys_remap_file_pages ----------------------------
long sys_remap_file_pages(unsigned long start, unsigned long size,
         unsigned long prot, unsigned long pgoff,
         unsigned long flags)
{
 NOT_IMPLEMENTED();
 return 0;
}

//-------------------------------- refrigerator --------------------------------
void refrigerator(unsigned long flags)
{
 NOT_IMPLEMENTED();
}

//-------------------- lower_zone_protection_sysctl_handler --------------------
int lower_zone_protection_sysctl_handler(struct ctl_table* p1,int p2,struct file* p3,void __user* p4,size_t* p5)
{
 NOT_IMPLEMENTED();
 return 0;
}

//------------------------------ blk_queue_bounce ------------------------------
void blk_queue_bounce(request_queue_t* q,struct bio** bio)
{
 NOT_IMPLEMENTED();
}

//----------------------- min_free_kbytes_sysctl_handler -----------------------
int min_free_kbytes_sysctl_handler(struct ctl_table *p1, int p2, struct file *p3,
               void __user *p4, size_t *p5)
{
 NOT_IMPLEMENTED();
 return 0;
}

//------------------------------ pci_find_device -------------------------------
struct pci_dev* pci_find_device(unsigned int vendor,unsigned int device,const struct pci_dev* from)
{
 NOT_IMPLEMENTED();
 return 0;
}

//-------------------------- mxcsr_feature_mask_init ---------------------------
void mxcsr_feature_mask_init(void)
{
 NOT_IMPLEMENTED();
}

//------------------------------ dmi_scan_machine ------------------------------
void __init dmi_scan_machine(void)
{
 NOT_IMPLEMENTED();
}

//------------------------------- get_smp_config -------------------------------
void __init get_smp_config(void)
{
 NOT_IMPLEMENTED();
}

//------------------------------ find_smp_config -------------------------------
void __init find_smp_config(void)
{
 NOT_IMPLEMENTED();
}

//---------------------------------- dump_fpu ----------------------------------
int dump_fpu(struct pt_regs *regs, struct user_i387_struct *fpu)
{
 NOT_IMPLEMENTED();
 return 0;
}

//------------------------------ do_notify_resume ------------------------------
__attribute__((regparm(3)))
 void do_notify_resume(struct pt_regs* regs,sigset_t *oldset
                       ,__u32 thread_info_flags)
{
 NOT_IMPLEMENTED();
}

//------------------------------ do_syscall_trace ------------------------------
__attribute__((regparm(3)))
 void do_syscall_trace(struct pt_regs* regs,int entryexit)
{
 NOT_IMPLEMENTED();
}

//---------------------------- disable_early_printk ----------------------------
void disable_early_printk(void)
{
 NOT_IMPLEMENTED();
}

//------------------------------- ptrace_disable -------------------------------
void ptrace_disable(struct task_struct *child)
{
 NOT_IMPLEMENTED();
}

//------------------------------- smp_cpus_done --------------------------------
void __init smp_cpus_done(unsigned int max_cpus)
{
 NOT_IMPLEMENTED();
}

//------------------------------- save_v86_state -------------------------------
struct pt_regs* fastcall save_v86_state(struct kernel_vm86_regs* regs)
{
 NOT_IMPLEMENTED();
 return 0;
}
