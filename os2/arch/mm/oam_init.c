/* $Id: oam_init.c,v 1.4 2005/07/11 22:18:31 smilcke Exp $ */

/*
 * oam_init.c
 * Autor:               Stefan Milcke
 * Erstellt am:         12.02.2005
 * Letzte Aenderung am: 07.07.2005
 *
*/

#include <lxcommon.h>

/*
 *  linux/arch/i386/mm/init.c
 *
 *  Copyright (C) 1995  Linus Torvalds
 *
 *  Support of BIGMEM added by Gerhard Wichert, Siemens AG, July 1999
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/ptrace.h>
#include <linux/mman.h>
#include <linux/mm.h>
#include <linux/hugetlb.h>
#include <linux/swap.h>
#include <linux/smp.h>
#include <linux/init.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>
#include <linux/bootmem.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/efi.h>

#include <asm/processor.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/dma.h>
#include <asm/fixmap.h>
#include <asm/e820.h>
#include <asm/apic.h>
#include <asm/tlb.h>
#include <asm/tlbflush.h>
#include <asm/sections.h>

unsigned long __PAGE_KERNEL = _PAGE_KERNEL;

DEFINE_PER_CPU(struct mmu_gather, mmu_gathers);
unsigned long highstart_pfn, highend_pfn;

kmem_cache_t *pgd_cache;
kmem_cache_t *pmd_cache;

//----------------------------- pgtable_cache_init -----------------------------
void __init pgtable_cache_init(void)
{
   if (PTRS_PER_PMD > 1) {
      pmd_cache = kmem_cache_create("pmd",
               PTRS_PER_PMD*sizeof(pmd_t),
               PTRS_PER_PMD*sizeof(pmd_t),
               0,
               pmd_ctor,
               NULL);
      if (!pmd_cache)
         panic("pgtable_cache_init(): cannot create pmd cache");
   }
   pgd_cache = kmem_cache_create("pgd",
            PTRS_PER_PGD*sizeof(pgd_t),
            PTRS_PER_PGD*sizeof(pgd_t),
            0,
            pgd_ctor,
            PTRS_PER_PMD == 1 ? pgd_dtor : NULL);
   if (!pgd_cache)
      panic("pgtable_cache_init(): Cannot create pgd cache");
}

//-------------------------------- free_initmem --------------------------------
void free_initmem(void)
{
}
