/* $Id: oo_miscw.c,v 1.13 2006/02/28 19:46:52 smilcke Exp $ */

/*
 * lx_miscw.c
 * Autor:               Stefan Milcke
 * Erstellt am:         30.06.2004
 * Letzte Aenderung am: 28.02.2006
 *
*/

#include <lxcommon.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/cpumask.h>
#include <linux/config.h>
#include <linux/sched.h>
#include <asm/processor.h>
#include <asm/i387.h>
#include <asm/uaccess.h>
#include <asm/desc.h>
#include <asm/io.h>
#include <asm/pgalloc.h>
#include <linux/slab.h>
#include <linux/shm.h>
#include <linux/mman.h>
#include <linux/pagemap.h>
#include <linux/swap.h>
#include <linux/syscalls.h>
#include <linux/init.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/personality.h>
#include <linux/security.h>
#include <linux/hugetlb.h>
#include <linux/profile.h>
#include <linux/module.h>
#include <linux/mount.h>

#include <asm/uaccess.h>
#include <asm/pgalloc.h>
#include <asm/tlb.h>
#include <linux/interrupt.h>
#include <linux/radix-tree.h>

int min_free_kbytes=0;
struct pglist_data contig_page_data;
int page_cluster=0;
int nr_swap_pages=0;
int block_dump=0;
atomic_t nr_pagecache;
atomic_t vm_committed_space;
int sysctl_lower_zone_protection=0;
int laptop_mode=0;
int errno=0;
int panic_timeout=100;
int panic_on_oops=0;
int tainted=0;
int smp_found_config=0;
unsigned int mca_pentium_flag;
unsigned long cache_decay_ticks=20;
unsigned long mmu_cr4_features=0;
struct console *console_drivers=0;
unsigned long __scheduling_functions_start_here;
unsigned long __scheduling_functions_end_here;
struct alt_instr* __alt_instructions=0;
struct alt_instr* __alt_instructions_end=0;

struct module __this_module;
unsigned int __initdata maxcpus=NR_CPUS;
int isa_dma_bridge_buggy;
int tsc_disable;
int pit_latch_buggy;
int smp_num_siblings=0;
struct Xgt_desc_struct idt_descr,cpu_gdt_descr[NR_CPUS];

//-------------------------------- sched_clock ---------------------------------
unsigned long long sched_clock(void)
{
 return jiffies_64 * (1000000000 / HZ);
}

//---------------------------------- *_alloca ----------------------------------
void *_alloca(size_t n)
{
 return kmalloc(n,GFP_KERNEL);
}

//------------------------------- out_of_memory --------------------------------
void out_of_memory(void)
{
}

//---------------------------------- mem_init ----------------------------------
void __init mem_init(void)
{
}

//---------------------------- build_all_zonelists -----------------------------
void __init build_all_zonelists(void)
{
}

//---------------------------- smp_prepare_boot_cpu ----------------------------
void __init smp_prepare_boot_cpu(void)
{
}

//------------------------------ page_alloc_init -------------------------------
void __init page_alloc_init(void)
{
}

//--------------------------- __buggy_fxsr_alignment ---------------------------
void __buggy_fxsr_alignment(void)
{
}

//------------------------------ smp_prepare_cpus ------------------------------
void __init smp_prepare_cpus(unsigned int max_cpus)
{
}

//----------------------- __you_cannot_kmalloc_that_much -----------------------
void __you_cannot_kmalloc_that_much(void)
{
 printk(KERN_EMERG "You cannot kmalloc that much\n");
}

//--------------------------- setup_profiling_timer ----------------------------
int setup_profiling_timer(unsigned int multiplier)
{
 NOT_IMPLEMENTED();
 return 0;
}

// TARGET_OS2_GNU2
/*
void __get_user_1(void)
{
 DevInt3();
}

void __get_user_2(void)
{
 DevInt3();
}

void __get_user_4(void)
{
 DevInt3();
}
*/

void __get_user_X(void)
{
 DevInt3();
}

long __get_user_bad(void)
{
 DevInt3();
 return 0;
}

void __this_fixmap_does_not_exist(void)
{
 DevInt3();
}

void _NSIG_WORDS_is_unsupported_size(void)
{
 DevInt3();
}

void __struct_cpy_bug(void)
{
 DevInt3();
}

void vide(void)
{
 DevInt3();
}

void efi_enter_virtual_mode(void)
{
 DevInt3();
}

// !TARGET_OS2_GNU2

#define TEXT_SEG  "CODE32"
#define DATA_SEG  "DATA32"

char _data[]=DATA_SEG;
char _sdata[]=DATA_SEG;
char __bss_start[]=TEXT_SEG;
char __bss_stop[]=TEXT_SEG;
char __init_begin[]=TEXT_SEG;
char __init_end[]=TEXT_SEG;
char _sinittext[]=TEXT_SEG;
char _einittext[]=TEXT_SEG;

struct exception_table_entry __start___ex_table[1]={{0}};
struct exception_table_entry __stop___ex_table[1]={{0}};
