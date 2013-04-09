/* $Id: om_page_alloc.c,v 1.13 2005/07/12 21:59:39 smilcke Exp $ */

/*
 * om_page_alloc.c
 * Autor:               Stefan Milcke
 * Erstellt am:         18.09.2004
 * Letzte Aenderung am: 12.07.2005
 *
*/

#define LXMALLOC_DECLARATIONS
#include <lxcommon.h>
/*
 *  linux/mm/page_alloc.c
 *
 *  Manages the free list, the system allocates free pages here.
 *  Note that kmalloc() lives in slab.c
 *
 *  Copyright (C) 1991, 1992, 1993, 1994  Linus Torvalds
 *  Swap reorganised 29.12.95, Stephen Tweedie
 *  Support of BIGMEM added by Gerhard Wichert, Siemens AG, July 1999
 *  Reshaped it to be a zoned allocator, Ingo Molnar, Red Hat, 1999
 *  Discontiguous memory support, Kanoj Sarcar, SGI, Nov 1999
 *  Zone balancing, Kanoj Sarcar, SGI, Jan 2000
 *  Per cpu hot/cold page lists, bulk allocation, Martin J. Bligh, Sept 2002
 *          (lots of bits borrowed from Ingo Molnar & Andrew Morton)
 */

#include <linux/config.h>
#include <linux/stddef.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/interrupt.h>
#include <linux/pagemap.h>
#include <linux/bootmem.h>
#include <linux/compiler.h>
#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/pagevec.h>
#include <linux/blkdev.h>
#include <linux/slab.h>
#include <linux/notifier.h>
#include <linux/topology.h>
#include <linux/sysctl.h>
#include <linux/cpu.h>

#include <asm/tlbflush.h>

#include <linux/mmzone.h>

#ifdef TARGET_OS2
#if 0
static struct pglist_data lx_pglist=
{
 .nr_zones=0,
 .node_mem_map=0,
 .bdata=0,
 .node_start_pfn=0,
 .node_present_pages=0,
 .node_spanned_pages=0,
 .node_id=0,
 .pgdat_next=0,
 .kswapd_wait=__WAIT_QUEUE_HEAD_INITIALIZER(lx_pglist.kswapd_wait),
 .kswapd=0
};

struct pglist_data *pgdat_list=&lx_pglist;
#else
struct pglist_data *pgdat_list=0;
#endif
#endif

unsigned long totalhigh_pages;

DEFINE_PER_CPU(struct page_state, page_states)={0};
#ifdef CONFIG_SMP
DEFINE_PER_CPU(long, nr_pagecache_local) = 0;
#endif

struct lx_pg_entry
{
 struct list_head list;
 unsigned int num_pages;
 unsigned long gfp_mask;
 unsigned long start;
 struct page* pages;
};

static LIST_HEAD(lx_pages);
static spinlock_t lx_pages_lock=SPIN_LOCK_UNLOCKED;

//----------------------------- LX_alloc_num_pages -----------------------------
struct page* fastcall
 LX_alloc_num_pages(unsigned int gfp_mask,unsigned int num_pages)
{
 struct lx_pg_entry* lpe=0;
 unsigned long f;
 int i;
 unsigned long sz=sizeof(struct lx_pg_entry)+(sizeof(struct page)*num_pages);
 lpe=(struct lx_pg_entry*)LX_malloc(sz);
 if(!lpe)
  return 0;
 lpe->start=(unsigned long)LX_malloc_gfp(PAGE_SIZE*num_pages,gfp_mask);
 if(lpe->start)
 {
  lpe->pages=(struct page*)(((unsigned long)lpe)+sizeof(struct lx_pg_entry));
  memset(lpe->pages,0,sizeof(struct page)*num_pages);
  lpe->num_pages=num_pages;
  lpe->gfp_mask=gfp_mask;
  for(i=0;i<lpe->num_pages;i++)
  {
   INIT_LIST_HEAD(&(lpe->pages[i].lru));
   atomic_set(&(lpe->pages[i].count),1);
   set_page_address(&(lpe->pages[i]),(void*)(lpe->start+(i*PAGE_SIZE)));
  }
  spin_lock_irqsave(&lx_pages_lock,f);
  list_add(&lpe->list,&lx_pages);
  spin_unlock_irqrestore(&lx_pages_lock,f);
  return lpe->pages;
 }
 LX_free(lpe);
 return 0;
}

//------------------------------- __alloc_pages --------------------------------
struct page* fastcall
  __alloc_pages(unsigned int gfp_mask,unsigned int order,struct zonelist* zonelist)
{
 return LX_alloc_num_pages(gfp_mask,1<<order);
}

//---------------------------- LX_get_lpe_from_page ----------------------------
static struct lx_pg_entry* LX_get_lpe_from_page(struct page* pg,int* index)
{
 struct list_head* lh;
 struct lx_pg_entry* lpe;
 list_for_each(lh,&lx_pages)
 {
  int i;
  lpe=list_entry(lh,struct lx_pg_entry,list);
  for(i=0;i<lpe->num_pages;i++)
  {
   if(&(lpe->pages[i])==pg)
   {
    *index=i;
    return lpe;
   }
  }
 }
 return 0;
}

//---------------------------- LX_get_lpe_from_addr ----------------------------
static struct lx_pg_entry* LX_get_lpe_from_addr(unsigned long addr,int* index)
{
 struct list_head* lh;
 struct lx_pg_entry* lpe;
 list_for_each(lh,&lx_pages)
 {
  int i;
  lpe=list_entry(lh,struct lx_pg_entry,list);
  for(i=0;i<lpe->num_pages;i++)
  {
   if(addr>=lpe->start+(i*PAGE_SIZE) && addr<lpe->start+((i+1)*PAGE_SIZE))
   {
    *index=i;
    return lpe;
   }
  }
 }
 return 0;
}

//------------------------------ __get_free_pages ------------------------------
fastcall unsigned long __get_free_pages(unsigned int gfp_mask, unsigned int order)
{
 unsigned long f;
 struct lx_pg_entry* lpe;
 struct page* pg;
 pg=__alloc_pages(gfp_mask,order,0);
 if(pg)
 {
  int index=0;
  spin_lock_irqsave(&lx_pages_lock,f);
  lpe=LX_get_lpe_from_page(pg,&index);
  spin_unlock_irqrestore(&lx_pages_lock,f);
  if(lpe)
   return lpe->start+(PAGE_SIZE*index);
 }
 return 0;
}

//-------------------------------- __free_pages --------------------------------
fastcall void __free_pages(struct page *page, unsigned int order)
{
 struct lx_pg_entry* lpe;
 unsigned long f;
 unsigned long num_pages=(1<<order);
 int index;
 int i=0;
 spin_lock_irqsave(&lx_pages_lock,f);
 lpe=LX_get_lpe_from_page(page,&index);
 if(lpe)
 {
  for(i=0;i<num_pages;i++)
   atomic_dec(&(lpe->pages[index+i].count));
  for(i=0;i<lpe->num_pages;i++)
   if(atomic_read(&(lpe->pages[i].count)))
    break;
  if(i>=lpe->num_pages)
   list_del(&lpe->list);
 }
 spin_unlock_irqrestore(&lx_pages_lock,f);
 if(lpe && i>=lpe->num_pages)
 {
  LX_free_gfp((void*)lpe->start);
  LX_free(lpe);
 }
}

EXPORT_SYMBOL(__free_pages);

//--------------------------------- free_pages ---------------------------------
fastcall void free_pages(unsigned long addr,unsigned int order)
{
 if(addr!=0)
 {
  BUG_ON(!virt_addr_valid(addr));
  __free_pages(virt_to_page(addr),order);
 }
}

EXPORT_SYMBOL(free_pages);

//------------------------------ LX_virt_to_page -------------------------------
struct page* LX_virt_to_page(unsigned long kaddr)
{
 struct lx_pg_entry* lpe;
 unsigned long f;
 int index;
 spin_lock_irqsave(&lx_pages_lock,f);
 lpe=LX_get_lpe_from_addr(kaddr,&index);
 spin_unlock_irqrestore(&lx_pages_lock,f);
 if(lpe)
  return &lpe->pages[index];
 return 0;
}

//----------------------------- LX_virt_addr_valid -----------------------------
int LX_virt_addr_valid(unsigned long kaddr)
{
 struct page* page=LX_virt_to_page(kaddr);
 if(page)
  return 1;
 return 0;
}

//------------------------------ get_zeroed_page -------------------------------
fastcall unsigned long get_zeroed_page(unsigned int gfp_mask)
{
 void* address;
 /*
  * get_zeroed_page() returns a 32-bit address, which cannot represent
  * a highmem page
  */
 BUG_ON(gfp_mask & __GFP_HIGHMEM);
 address=(void*)__get_free_pages(gfp_mask,0);
 if (address) {
  clear_page(address);
  return (unsigned long) address;
 }
 return 0;
}

EXPORT_SYMBOL(get_zeroed_page);

#ifdef TARGET_OS2
extern unsigned long lx_totavail_pages;
#endif // TARGET_OS2
/*
 * Total amount of free (allocatable) RAM:
 */
//------------------------------- nr_free_pages --------------------------------
unsigned int nr_free_pages(void)
{
#ifdef TARGET_OS2
 return lx_totavail_pages;
#else
   unsigned int sum = 0;
   struct zone *zone;

   for_each_zone(zone)
      sum += zone->free_pages;

 return sum;
#endif
}

/*
 * Amount of free RAM allocatable within ZONE_DMA and ZONE_NORMAL
 */
//---------------------------- nr_free_buffer_pages ----------------------------
unsigned int nr_free_buffer_pages(void)
{
#ifdef TARGET_OS2
 return nr_free_pages()/4;
#else
 return nr_free_zone_pages(GFP_USER & GFP_ZONEMASK);
#endif
}

//------------------------------ __get_page_state ------------------------------
void __get_page_state(struct page_state* ret,int nr)
{
   int cpu = 0;

   memset(ret, 0, sizeof(*ret));
   while (cpu < NR_CPUS) {
      unsigned long *in, *out, off;

      if (!cpu_possible(cpu)) {
         cpu++;
         continue;
      }

      in = (unsigned long *)&per_cpu(page_states, cpu);
      cpu++;
      if (cpu < NR_CPUS && cpu_possible(cpu))
         prefetch(&per_cpu(page_states, cpu));
      out = (unsigned long *)ret;
      for (off = 0; off < nr; off++)
         *out++ += *in++;
   }
}

//------------------------------- get_page_state -------------------------------
void get_page_state(struct page_state *ret)
{
   int nr;

   nr = offsetof(struct page_state, GET_PAGE_STATE_LAST);
   nr /= sizeof(unsigned long);

   __get_page_state(ret, nr + 1);
}

//---------------------------- get_full_page_state -----------------------------
void get_full_page_state(struct page_state *ret)
{
   __get_page_state(ret, sizeof(*ret) / sizeof(unsigned long));
}

//----------------------------- nr_used_zone_pages -----------------------------
unsigned int nr_used_zone_pages(void)
{
#ifdef TARGET_OS2
 return 0;
#else
   unsigned int pages = 0;
   struct zone *zone;

   for_each_zone(zone)
      pages += zone->nr_active + zone->nr_inactive;

   return pages;
#endif
}

/*
 * Amount of free RAM allocatable within all zones
 */
//-------------------------- nr_free_pagecache_pages ---------------------------
unsigned int nr_free_pagecache_pages(void)
{
#ifdef TARGET_OS2
 return 100;
#else
   return nr_free_zone_pages(GFP_HIGHUSER & GFP_ZONEMASK);
#endif
}

//------------------------------ get_zone_counts -------------------------------
void get_zone_counts(unsigned long* active,unsigned long* inactive,unsigned long* free)
{
 NOT_IMPLEMENTED();
 DevInt3();
}

//--------------------------------- si_meminfo ---------------------------------
void si_meminfo(struct sysinfo *val)
{
   val->totalram = totalram_pages;
   val->sharedram = 0;
   val->freeram = nr_free_pages();
   val->bufferram = nr_blockdev_pages();
#ifdef CONFIG_HIGHMEM
   val->totalhigh = totalhigh_pages;
   val->freehigh = nr_free_highpages();
#else
   val->totalhigh = 0;
   val->freehigh = 0;
#endif
   val->mem_unit = PAGE_SIZE;
}

EXPORT_SYMBOL(si_meminfo);

#ifdef CONFIG_NUMA
static void show_node(struct zone *zone)
{
   printk("Node %d ", zone->zone_pgdat->node_id);
}
#else
#define show_node(zone) do { } while (0)
#endif

#define K(x) ((x) << (PAGE_SHIFT-10))


//------------------------------ show_free_areas -------------------------------
void show_free_areas(void)
{
   struct page_state ps;
   int cpu, temperature;
   unsigned long active;
   unsigned long inactive;
   unsigned long free;
   struct zone *zone;

   for_each_zone(zone) {
      show_node(zone);
      printk("%s per-cpu:", zone->name);

      if (!zone->present_pages) {
         printk(" empty\n");
         continue;
      } else
         printk("\n");

      for (cpu = 0; cpu < NR_CPUS; ++cpu) {
         struct per_cpu_pageset *pageset;

         if (!cpu_possible(cpu))
            continue;

         pageset = zone->pageset + cpu;

         for (temperature = 0; temperature < 2; temperature++)
            printk("cpu %d %s: low %d, high %d, batch %d\n",
               cpu,
               temperature ? "cold" : "hot",
               pageset->pcp[temperature].low,
               pageset->pcp[temperature].high,
               pageset->pcp[temperature].batch);
      }
   }

   get_page_state(&ps);
   get_zone_counts(&active, &inactive, &free);

   printk("\nFree pages: %11ukB (%ukB HighMem)\n",
      K(nr_free_pages()),
      K(nr_free_highpages()));

   printk("Active:%lu inactive:%lu dirty:%lu writeback:%lu "
      "unstable:%lu free:%u slab:%lu mapped:%lu pagetables:%lu\n",
      active,
      inactive,
      ps.nr_dirty,
      ps.nr_writeback,
      ps.nr_unstable,
      nr_free_pages(),
      ps.nr_slab,
      ps.nr_mapped,
      ps.nr_page_table_pages);

   for_each_zone(zone) {
      int i;

      show_node(zone);
      printk("%s"
         " free:%lukB"
         " min:%lukB"
         " low:%lukB"
         " high:%lukB"
         " active:%lukB"
         " inactive:%lukB"
         " present:%lukB"
         "\n",
         zone->name,
         K(zone->free_pages),
         K(zone->pages_min),
         K(zone->pages_low),
         K(zone->pages_high),
         K(zone->nr_active),
         K(zone->nr_inactive),
         K(zone->present_pages)
         );
      printk("protections[]:");
      for (i = 0; i < MAX_NR_ZONES; i++)
         printk(" %lu", zone->protection[i]);
      printk("\n");
   }

   for_each_zone(zone) {
      struct list_head *elem;
      unsigned long nr, flags, order, total = 0;

      show_node(zone);
      printk("%s: ", zone->name);
      if (!zone->present_pages) {
         printk("empty\n");
         continue;
      }

      spin_lock_irqsave(&zone->lock, flags);
      for (order = 0; order < MAX_ORDER; order++) {
         nr = 0;
         list_for_each(elem, &zone->free_area[order].free_list)
            ++nr;
         total += nr << order;
         printk("%lu*%lukB ", nr, K(1UL) << order);
      }
      spin_unlock_irqrestore(&zone->lock, flags);
      printk("= %lukB\n", K(total));
   }

   show_swap_cache_info();
}

#ifdef CONFIG_PROC_FS

#include <linux/seq_file.h>

static void *frag_start(struct seq_file *m, loff_t *pos)
{
   pg_data_t *pgdat;
   loff_t node = *pos;

   for (pgdat = pgdat_list; pgdat && node; pgdat = pgdat->pgdat_next)
      --node;

   return pgdat;
}

static void *frag_next(struct seq_file *m, void *arg, loff_t *pos)
{
   pg_data_t *pgdat = (pg_data_t *)arg;

   (*pos)++;
   return pgdat->pgdat_next;
}

static void frag_stop(struct seq_file *m, void *arg)
{
}

/*
 * This walks the freelist for each zone. Whilst this is slow, I'd rather
 * be slow here than slow down the fast path by keeping stats - mjbligh
 */
static int frag_show(struct seq_file *m, void *arg)
{
   pg_data_t *pgdat = (pg_data_t *)arg;
   struct zone *zone;
   struct zone *node_zones = pgdat->node_zones;
   unsigned long flags;
   int order;

   for (zone = node_zones; zone - node_zones < MAX_NR_ZONES; ++zone) {
      if (!zone->present_pages)
         continue;

      spin_lock_irqsave(&zone->lock, flags);
      seq_printf(m, "Node %d, zone %8s ", pgdat->node_id, zone->name);
      for (order = 0; order < MAX_ORDER; ++order) {
         unsigned long nr_bufs = 0;
         struct list_head *elem;

         list_for_each(elem, &(zone->free_area[order].free_list))
            ++nr_bufs;
         seq_printf(m, "%6lu ", nr_bufs);
      }
      spin_unlock_irqrestore(&zone->lock, flags);
      seq_putc(m, '\n');
   }
   return 0;
}

struct seq_operations fragmentation_op = {
   .start   = frag_start,
   .next = frag_next,
   .stop = frag_stop,
   .show = frag_show,
};

static char *vmstat_text[] = {
   "nr_dirty",
   "nr_writeback",
   "nr_unstable",
   "nr_page_table_pages",
   "nr_mapped",
   "nr_slab",

   "pgpgin",
   "pgpgout",
   "pswpin",
   "pswpout",
   "pgalloc_high",

   "pgalloc_normal",
   "pgalloc_dma",
   "pgfree",
   "pgactivate",
   "pgdeactivate",

   "pgfault",
   "pgmajfault",
   "pgrefill_high",
   "pgrefill_normal",
   "pgrefill_dma",

   "pgsteal_high",
   "pgsteal_normal",
   "pgsteal_dma",
   "pgscan_kswapd_high",
   "pgscan_kswapd_normal",

   "pgscan_kswapd_dma",
   "pgscan_direct_high",
   "pgscan_direct_normal",
   "pgscan_direct_dma",
   "pginodesteal",

   "slabs_scanned",
   "kswapd_steal",
   "kswapd_inodesteal",
   "pageoutrun",
   "allocstall",

   "pgrotated",
};

static void *vmstat_start(struct seq_file *m, loff_t *pos)
{
   struct page_state *ps;

   if (*pos >= ARRAY_SIZE(vmstat_text))
      return NULL;

   ps = kmalloc(sizeof(*ps), GFP_KERNEL);
   m->private = ps;
   if (!ps)
      return ERR_PTR(-ENOMEM);
   get_full_page_state(ps);
   ps->pgpgin /= 2;     /* sectors -> kbytes */
   ps->pgpgout /= 2;
   return (unsigned long *)ps + *pos;
}

static void *vmstat_next(struct seq_file *m, void *arg, loff_t *pos)
{
   (*pos)++;
   if (*pos >= ARRAY_SIZE(vmstat_text))
      return NULL;
   return (unsigned long *)m->private + *pos;
}

static int vmstat_show(struct seq_file *m, void *arg)
{
   unsigned long *l = arg;
   unsigned long off = l - (unsigned long *)m->private;

   seq_printf(m, "%s %lu\n", vmstat_text[off], *l);
   return 0;
}

static void vmstat_stop(struct seq_file *m, void *arg)
{
   kfree(m->private);
   m->private = NULL;
}

struct seq_operations vmstat_op = {
   .start   = vmstat_start,
   .next = vmstat_next,
   .stop = vmstat_stop,
   .show = vmstat_show,
};


#endif /* CONFIG_PROC_FS */
