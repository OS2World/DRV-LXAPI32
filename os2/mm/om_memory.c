/* $Id: om_memory.c,v 1.5 2005/07/16 22:44:17 smilcke Exp $ */

/*
 * memory.c
 * Autor:               Stefan Milcke
 * Erstellt am:         16.06.2004
 * Letzte Aenderung am: 12.07.2005
 *
*/

#include <lxcommon.h>
#include <linux/kernel_stat.h>
#include <linux/mm.h>
#include <linux/hugetlb.h>
#include <linux/mman.h>
#include <linux/swap.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>
// #include <linux/rmap.h>
#include <linux/module.h>
#include <linux/init.h>

#include <asm/pgalloc.h>
// #include <asm/rmap.h>
#include <asm/uaccess.h>
#include <asm/tlb.h>
#include <asm/tlbflush.h>
#include <asm/pgtable.h>

// #include <linux/swapops.h>
#include <linux/elf.h>

/*
 *  linux/mm/memory.c
 *
 *  Copyright (C) 1991, 1992, 1993, 1994  Linus Torvalds
 */

/*
 * demand-loading started 01.12.91 - seems it is high on the list of
 * things wanted, and it should be easy to implement. - Linus
 */

/*
 * Ok, demand-loading was easy, shared pages a little bit tricker. Shared
 * pages started 02.12.91, seems to work. - Linus.
 *
 * Tested sharing by executing about 30 /bin/sh: under the old kernel it
 * would have taken more than the 6M I have free, but it worked well as
 * far as I could see.
 *
 * Also corrected some "invalidate()"s - I wasn't doing enough of them.
 */

/*
 * Real VM (paging to/from disk) started 18.12.91. Much more work and
 * thought has to go into this. Oh, well..
 * 19.12.91  -  works, somewhat. Sometimes I get faults, don't know why.
 *    Found it. Everything seems to work now.
 * 20.12.91  -  Ok, making the swap-device changeable like the root.
 */

/*
 * 05.04.94  -  Multi-page memory management added for v1.1.
 *       Idea by Alex Bligh (alex@cconcepts.co.uk)
 *
 * 16.07.99  -  Support of BIGMEM added by Gerhard Wichert, Siemens AG
 *    (Gerhard.Wichert@pdb.siemens.de)
 */

#ifndef CONFIG_DISCONTIGMEM
/* use the per-pgdat data instead for discontigmem - mbligh */
unsigned long max_mapnr;
struct page *mem_map;

EXPORT_SYMBOL(max_mapnr);
EXPORT_SYMBOL(mem_map);
#endif

unsigned long num_physpages;
void * high_memory;
struct page *highmem_start_page;

EXPORT_SYMBOL(num_physpages);
EXPORT_SYMBOL(highmem_start_page);
EXPORT_SYMBOL(high_memory);

/*
 * We special-case the C-O-W ZERO_PAGE, because it's such
 * a common occurrence (no need to read the page to know
 * that it's zero - better for the cache and memory subsystem).
 */
static inline void copy_cow_page(struct page * from, struct page * to, unsigned long address)
{
 DevInt3();
}

/*
 * Note: this doesn't free the actual pages themselves. That
 * has been handled earlier when unmapping all the memory regions.
 */
static inline void free_one_pmd(struct mmu_gather *tlb, pmd_t * dir)
{
 DevInt3();
}

static inline void free_one_pgd(struct mmu_gather *tlb, pgd_t * dir)
{
 DevInt3();
}

/*
 * This function clears all user-level page tables of a process - this
 * is needed by execve(), so that old pages aren't in the way.
 *
 * Must be called with pagetable lock held.
 */
void clear_page_tables(struct mmu_gather *tlb, unsigned long first, int nr)
{
 DevInt3();
}

pte_t fastcall * pte_alloc_map(struct mm_struct *mm, pmd_t *pmd, unsigned long address)
{
 DevInt3();
 return 0;
}

pte_t fastcall * pte_alloc_kernel(struct mm_struct *mm, pmd_t *pmd, unsigned long address)
{
 DevInt3();
 return 0;
}
#define PTE_TABLE_MASK  ((PTRS_PER_PTE-1) * sizeof(pte_t))
#define PMD_TABLE_MASK  ((PTRS_PER_PMD-1) * sizeof(pmd_t))

/*
 * copy one vm_area from one task to the other. Assumes the page tables
 * already present in the new task to be cleared in the whole range
 * covered by this vma.
 *
 * 08Jan98 Merged into one routine from several inline routines to reduce
 *         variable count and make things faster. -jj
 *
 * dst->page_table_lock is held on entry and exit,
 * but may be dropped within pmd_alloc() and pte_alloc_map().
 */
int copy_page_range(struct mm_struct *dst, struct mm_struct *src,
         struct vm_area_struct *vma)
{
 DevInt3();
 return 0;
}

/* Dispose of an entire struct mmu_gather per rescheduling point */
#if defined(CONFIG_SMP) && defined(CONFIG_PREEMPT)
#define ZAP_BLOCK_SIZE  (FREE_PTE_NR * PAGE_SIZE)
#endif

/* For UP, 256 pages at a time gives nice low latency */
#if !defined(CONFIG_SMP) && defined(CONFIG_PREEMPT)
#define ZAP_BLOCK_SIZE  (256 * PAGE_SIZE)
#endif

/* No preempt: go for improved straight-line efficiency */
#if !defined(CONFIG_PREEMPT)
#define ZAP_BLOCK_SIZE  (1024 * PAGE_SIZE)
#endif

/**
 * unmap_vmas - unmap a range of memory covered by a list of vma's
 * @tlbp: address of the caller's struct mmu_gather
 * @mm: the controlling mm_struct
 * @vma: the starting vma
 * @start_addr: virtual address at which to start unmapping
 * @end_addr: virtual address at which to end unmapping
 * @nr_accounted: Place number of unmapped pages in vm-accountable vma's here
 * @details: details of nonlinear truncation or shared cache invalidation
 *
 * Returns the number of vma's which were covered by the unmapping.
 *
 * Unmap all pages in the vma list.  Called under page_table_lock.
 *
 * We aim to not hold page_table_lock for too long (for scheduling latency
 * reasons).  So zap pages in ZAP_BLOCK_SIZE bytecounts.  This means we need to
 * return the ending mmu_gather to the caller.
 *
 * Only addresses between `start' and `end' will be unmapped.
 *
 * The VMA list must be sorted in ascending virtual address order.
 *
 * unmap_vmas() assumes that the caller will flush the whole unmapped address
 * range after unmap_vmas() returns.  So the only responsibility here is to
 * ensure that any thus-far unmapped pages are flushed before unmap_vmas()
 * drops the lock and schedules.
 */
int unmap_vmas(struct mmu_gather **tlbp, struct mm_struct *mm,
      struct vm_area_struct *vma, unsigned long start_addr,
      unsigned long end_addr, unsigned long *nr_accounted,
      struct zap_details *details)
{
 DevInt3();
 return 0;
}

/**
 * zap_page_range - remove user pages in a given range
 * @vma: vm_area_struct holding the applicable pages
 * @address: starting address of pages to zap
 * @size: number of bytes to zap
 * @details: details of nonlinear truncation or shared cache invalidation
 */
void zap_page_range(struct vm_area_struct *vma, unsigned long address,
      unsigned long size, struct zap_details *details)
{
 DevInt3();
}

/*
 * Do a quick page-table lookup for a single page.
 * mm->page_table_lock must be held.
 */
struct page *
follow_page(struct mm_struct *mm, unsigned long address, int write)
{
 DevInt3();
 return 0;
}

/*
 * Given a physical address, is there a useful struct page pointing to
 * it?  This may become more complex in the future if we start dealing
 * with IO-aperture pages for direct-IO.
 */

static inline struct page *get_page_map(struct page *page)
{
 DevInt3();
 return 0;
}


static inline int
untouched_anonymous_page(struct mm_struct* mm, struct vm_area_struct *vma,
          unsigned long address)
{
 DevInt3();
 return 0;
}


int get_user_pages(struct task_struct *tsk, struct mm_struct *mm,
      unsigned long start, int len, int write, int force,
      struct page **pages, struct vm_area_struct **vmas)
{
 DevInt3();
 return 0;
}

EXPORT_SYMBOL(get_user_pages);

static inline int zeromap_pmd_range(struct mm_struct *mm, pmd_t * pmd, unsigned long address,
                                    unsigned long size, pgprot_t prot)
{
 DevInt3();
 return 0;
}

int zeromap_page_range(struct vm_area_struct *vma, unsigned long address, unsigned long size, pgprot_t prot)
{
 DevInt3();
 return 0;
}

/*
 * maps a range of physical memory into the requested pages. the old
 * mappings are removed. any references to nonexistent pages results
 * in null mappings (currently treated as "copy-on-access")
 */
static inline void remap_pte_range(pte_t * pte, unsigned long address, unsigned long size,
   unsigned long phys_addr, pgprot_t prot)
{
 DevInt3();
}

static inline int remap_pmd_range(struct mm_struct *mm, pmd_t * pmd, unsigned long address, unsigned long size,
   unsigned long phys_addr, pgprot_t prot)
{
 DevInt3();
 return 0;
}

/*  Note: this is only safe if the mm semaphore is held when called. */
int remap_page_range(struct vm_area_struct *vma, unsigned long from, unsigned long phys_addr, unsigned long size, pgprot_t prot)
{
 DevInt3();
 return 0;
}

EXPORT_SYMBOL(remap_page_range);

/*
 * We hold the mm semaphore for reading and vma->vm_mm->page_table_lock
 */
static inline void break_cow(struct vm_area_struct * vma, struct page * new_page, unsigned long address,
      pte_t *page_table)
{
 DevInt3();
}

/*
 * Helper function for unmap_mapping_range().
 */
static void unmap_mapping_range_list(struct list_head *head,
                 struct zap_details *details)
{
   struct vm_area_struct *vma;
   pgoff_t vba, vea, zba, zea;

   list_for_each_entry(vma, head, shared) {
      if (unlikely(vma->vm_flags & VM_NONLINEAR)) {
         details->nonlinear_vma = vma;
         zap_page_range(vma, vma->vm_start,
            vma->vm_end - vma->vm_start, details);
         details->nonlinear_vma = NULL;
         continue;
      }
      vba = vma->vm_pgoff;
      vea = vba + ((vma->vm_end - vma->vm_start) >> PAGE_SHIFT) - 1;
      /* Assume for now that PAGE_CACHE_SHIFT == PAGE_SHIFT */
      if (vba > details->last_index || vea < details->first_index)
         continue;   /* Mapping disjoint from hole. */
      zba = details->first_index;
      if (zba < vba)
         zba = vba;
      zea = details->last_index;
      if (zea > vea)
         zea = vea;
      zap_page_range(vma,
         ((zba - vba) << PAGE_SHIFT) + vma->vm_start,
         (zea - zba + 1) << PAGE_SHIFT,
         details->check_mapping? details: NULL);
   }
}

/**
 * unmap_mapping_range - unmap the portion of all mmaps
 * in the specified address_space corresponding to the specified
 * page range in the underlying file.
 * @address_space: the address space containing mmaps to be unmapped.
 * @holebegin: byte in first page to unmap, relative to the start of
 * the underlying file.  This will be rounded down to a PAGE_SIZE
 * boundary.  Note that this is different from vmtruncate(), which
 * must keep the partial page.  In contrast, we must get rid of
 * partial pages.
 * @holelen: size of prospective hole in bytes.  This will be rounded
 * up to a PAGE_SIZE boundary.  A holelen of zero truncates to the
 * end of the file.
 * @even_cows: 1 when truncating a file, unmap even private COWed pages;
 * but 0 when invalidating pagecache, don't throw away private data.
 */
void unmap_mapping_range(struct address_space *mapping,
   loff_t const holebegin, loff_t const holelen, int even_cows)
{
   struct zap_details details;
   pgoff_t hba = holebegin >> PAGE_SHIFT;
   pgoff_t hlen = (holelen + PAGE_SIZE - 1) >> PAGE_SHIFT;

   /* Check for overflow. */
   if (sizeof(holelen) > sizeof(hlen)) {
      long long holeend =
         (holebegin + holelen + PAGE_SIZE - 1) >> PAGE_SHIFT;
      if (holeend & ~(long long)ULONG_MAX)
         hlen = ULONG_MAX - hba + 1;
   }

   details.check_mapping = even_cows? NULL: mapping;
   details.nonlinear_vma = NULL;
   details.first_index = hba;
   details.last_index = hba + hlen - 1;
   if (details.last_index < details.first_index)
      details.last_index = ULONG_MAX;

   down(&mapping->i_shared_sem);
   /* Protect against page fault */
   atomic_inc(&mapping->truncate_count);
   if (unlikely(!list_empty(&mapping->i_mmap)))
      unmap_mapping_range_list(&mapping->i_mmap, &details);

   /* Don't waste time to check mapping on fully shared vmas */
   details.check_mapping = NULL;

   if (unlikely(!list_empty(&mapping->i_mmap_shared)))
      unmap_mapping_range_list(&mapping->i_mmap_shared, &details);
   up(&mapping->i_shared_sem);
}
EXPORT_SYMBOL(unmap_mapping_range);

/*
 * Handle all mappings that got truncated by a "truncate()"
 * system call.
 *
 * NOTE! We have to be ready to update the memory sharing
 * between the file and the memory map for a potential last
 * incomplete page.  Ugly, but necessary.
 */
int vmtruncate(struct inode * inode, loff_t offset)
{
   struct address_space *mapping = inode->i_mapping;
   unsigned long limit;

   if (inode->i_size < offset)
      goto do_expand;
   i_size_write(inode, offset);
   unmap_mapping_range(mapping, offset + PAGE_SIZE - 1, 0, 1);
   truncate_inode_pages(mapping, offset);
   goto out_truncate;

do_expand:
   limit = current->rlim[RLIMIT_FSIZE].rlim_cur;
   if (limit != RLIM_INFINITY && offset > limit)
      goto out_sig;
   if (offset > inode->i_sb->s_maxbytes)
      goto out;
   i_size_write(inode, offset);

out_truncate:
   if (inode->i_op && inode->i_op->truncate)
      inode->i_op->truncate(inode);
   return 0;
out_sig:
   send_sig(SIGXFSZ, current, 0);
out:
   return -EFBIG;
}

EXPORT_SYMBOL(vmtruncate);

/*
 * Primitive swap readahead code. We simply read an aligned block of
 * (1 << page_cluster) entries in the swap area. This method is chosen
 * because it doesn't cost us any seek time.  We also make sure to queue
 * the 'original' request together with the readahead ones...
 */
void swapin_readahead(swp_entry_t entry)
{
 DevInt3();
}

/*
 * These routines also need to handle stuff like marking pages dirty
 * and/or accessed for architectures that don't do it in hardware (most
 * RISC architectures).  The early dirtying is also good on the i386.
 *
 * There is also a hook called "update_mmu_cache()" that architectures
 * with external mmu caches can use to update those (ie the Sparc or
 * PowerPC hashed page tables that act as extended TLBs).
 *
 * Note the "page_table_lock". It is to protect against kswapd removing
 * pages from under us. Note that kswapd only ever _removes_ pages, never
 * adds them. As such, once we have noticed that the page is not present,
 * we can drop the lock early.
 *
 * The adding of pages is protected by the MM semaphore (which we hold),
 * so we don't need to worry about a page being suddenly been added into
 * our VM.
 *
 * We enter with the pagetable spinlock held, we are supposed to
 * release it when done.
 */
static inline int handle_pte_fault(struct mm_struct *mm,
   struct vm_area_struct * vma, unsigned long address,
   int write_access, pte_t *pte, pmd_t *pmd)
{
 DevInt3();
 return 0;
}

/*
 * By the time we get here, we already hold the mm semaphore
 */
int handle_mm_fault(struct mm_struct *mm, struct vm_area_struct * vma,
   unsigned long address, int write_access)
{
 DevInt3();
 return 0;
}

/*
 * Allocate page middle directory.
 *
 * We've already handled the fast-path in-line, and we own the
 * page table lock.
 *
 * On a two-level page table, this ends up actually being entirely
 * optimized away.
 */
pmd_t fastcall *__pmd_alloc(struct mm_struct *mm, pgd_t *pgd, unsigned long address)
{
 DevInt3();
 return 0;
}

int make_pages_present(unsigned long addr, unsigned long end)
{
 DevInt3();
 return 0;
}

/*
 * Map a vmalloc()-space virtual address to the physical page.
 */
struct page * vmalloc_to_page(void * vmalloc_addr)
{
 DevInt3();
 return 0;
}

EXPORT_SYMBOL(vmalloc_to_page);

#if !defined(CONFIG_ARCH_GATE_AREA)

#if defined(AT_SYSINFO_EHDR)
struct vm_area_struct gate_vma;

static int __init gate_vma_init(void)
{
   gate_vma.vm_mm = NULL;
   gate_vma.vm_start = FIXADDR_USER_START;
   gate_vma.vm_end = FIXADDR_USER_END;
   gate_vma.vm_page_prot = PAGE_READONLY;
   gate_vma.vm_flags = 0;
   return 0;
}
__initcall(gate_vma_init);
#endif

struct vm_area_struct *get_gate_vma(struct task_struct *tsk)
{
 DevInt3();
 return 0;
}

int in_gate_area(struct task_struct *task, unsigned long addr)
{
 DevInt3();
 return 0;
}

#endif
