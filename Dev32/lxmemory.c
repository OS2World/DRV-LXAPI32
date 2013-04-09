/* $Id: lxmemory.c,v 1.25 2006/04/27 21:17:51 smilcke Exp $ */

/*
 * malloc.c
 * Autor:               Stefan Milcke
 * Erstellt am:         26.10.2001
 * Letzte Aenderung am: 25.04.2006
 *
*/

#define LXMALLOC_DECLARATIONS
#include <lxcommon.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <asm/page.h>
#include <asm/atomic.h>
#include <linux/moduleparam.h>

extern WORD32 GDTSelectors_PTR;

static unsigned long selectorsUsed[MAX_GDTSELECTORS]={0};
static unsigned short* GDTSelectors=0;
extern unsigned long lx_max_alloc_aligned_retry;

//------------------------- lx_set_alloc_aligned_retry -------------------------
static int __init lx_set_alloc_aligned_retry(char* str)
{
 int a=100;
 get_option(&str,&a);
 if(a>=10 && a<=800)
  lx_max_alloc_aligned_retry=a;
 return 1;
}
__setup("lx_set_alloc_aligned_retry=", lx_set_alloc_aligned_retry);

//---------------------------- LX_get_gdt_selectors ----------------------------
static inline void LX_get_gdt_selectors(void)
{
 if(!GDTSelectors)
 {
  DevVirtToLin(SELECTOROF(GDTSelectors_PTR)
               ,OFFSETOF(GDTSelectors_PTR)
               ,(LINEAR*)&GDTSelectors);
 }
}

//---------------------------- LX_allocGDTSelector -----------------------------
unsigned short LX_allocGDTSelector(void)
{
 int i;
 LX_get_gdt_selectors();
 for(i=0;i<MAX_GDTSELECTORS;i++)
 {
  if(!selectorsUsed[i])
  {
   selectorsUsed[i]=1;
   return GDTSelectors[i];
  }
 }
 return 0;
}

//----------------------------- LX_freeGDTSelector -----------------------------
int LX_freeGDTSelector(unsigned short selector)
{
 int i;
 LX_get_gdt_selectors();
 for(i=0;i<MAX_GDTSELECTORS;i++)
 {
  if(GDTSelectors[i]==selector)
  {
   selectorsUsed[i]=0;
   return TRUE;
  }
 }
 return FALSE;
}

#define LXMT_UNKNOWN       (0)   // unknown
#define LXMT_NORMAL_MALLOC (1)   // Normally allocated via LX_malloc
#define LXMT_PHYS_TO_VIRT  (2)   // Mapped from physical to virtual
#define LXMT_VIRT_TO_PHYS  (3)   // Mapped from virtual to physical

struct lx_memobj
{
 struct list_head list;
 void* virtAddr;
 void* physAddr;
 void* unalignedAddr;
 unsigned long size;
 unsigned long realsize;
 unsigned short memflags;
 unsigned short type;
 unsigned short tid;
 unsigned short pid;
};

static spinlock_t memlist_lock=SPIN_LOCK_UNLOCKED;
static LIST_HEAD(memobjs);

//------------------------------ LX_alloc_memobj -------------------------------
// inline ?
static struct lx_memobj* LX_alloc_memobj(void)
{
 struct lx_memobj* pm;
 if(DevVMAlloc(lx_memobj_malloc_flags | VMDHA_USEHIGHMEM
               ,sizeof(struct lx_memobj),(LINEAR)-1,(LINEAR*)&pm))
  if(DevVMAlloc(lx_memobj_malloc_flags
                ,sizeof(struct lx_memobj),(LINEAR)-1,(LINEAR*)&pm))
   return NULL;
 pm->tid=lx_current_tid;
 pm->pid=lx_current_pid;
 pm->type=LXMT_UNKNOWN;
 return pm;
}

//------------------------------- LX_find_memobj -------------------------------
// inline ?
static struct lx_memobj* LX_find_memobj(void* addr,int bPhysAddr)
{
 struct list_head* lh;
 struct lx_memobj* pm;
 if(bPhysAddr)
 {
  list_for_each(lh,&memobjs)
  {
   pm=list_entry(lh,struct lx_memobj,list);
   if(pm->physAddr==addr)
    return pm;
  }
 }
 else
 {
  list_for_each(lh,&memobjs)
  {
   pm=list_entry(lh,struct lx_memobj,list);
   if(pm->virtAddr==addr)
    return pm;
  }
 }
 return NULL;
}

//--------------------------------- __LX_free ----------------------------------
void __LX_free(void *ptr)
{
 unsigned long f;
 struct lx_memobj* pm;
 spin_lock_irqsave(&memlist_lock,f);
 pm=LX_find_memobj(ptr,0);
 if(pm)
 {
  if(pm->type==LXMT_NORMAL_MALLOC || pm->type==LXMT_PHYS_TO_VIRT)
   list_del(&pm->list);
  else
   pm=0;
 }
 spin_unlock_irqrestore(&memlist_lock,f);
 if(pm)
 {
  DevVMFree((LINEAR)pm->unalignedAddr);
  DevVMFree((LINEAR)pm);
 }
 else
  DevVMFree((LINEAR)ptr);
}

//-------------------------------- __LX_malloc ---------------------------------
void* __LX_malloc(unsigned long size,unsigned long flags)
{
 unsigned long f;
 struct lx_memobj* pm=LX_alloc_memobj();
 if(!pm)
  return NULL;
 pm->memflags=(unsigned short)flags;
 pm->size=size;
 pm->physAddr=0;
 pm->type=LXMT_NORMAL_MALLOC;
/* Try to allocate aligned memory.
   The allocation is aligned to ALLOC_ALIGNED_ALIGN (Normally THREAD_SIZE)
   If size smaller than PAGE_SIZE, the returned memory object is always unaligned!
 */
 pm->virtAddr=(void*)LXA_VMAllocAligned((unsigned long)pm->memflags,pm->size
                                        ,(unsigned long*)&pm->virtAddr
                                        ,(unsigned long*)&pm->realsize
                                        ,(unsigned long*)&pm->unalignedAddr);
 if(!pm->virtAddr)
 {
  DevVMFree((LINEAR)pm);
  return NULL;
 }
 spin_lock_irqsave(&memlist_lock,f);
 list_add(&pm->list,&memobjs);
 spin_unlock_irqrestore(&memlist_lock,f);
 return pm->virtAddr;
}

//------------------------------ __LX_malloc_gfp -------------------------------
void* __LX_malloc_gfp(unsigned long size,unsigned long gfp_mask)
{
 int mflags=lx_default_malloc_flags;
 void* p=0;
 if(gfp_mask&__GFP_DMA)
  mflags|=VMDHA_16M | VMDHA_CONTIG | VMDHA_FIXED;
 else
  mflags|=VMDHA_USEHMA;
/* VMDHA_USEHIGHMEM only valid at INIT time ???
 if(gfp_mask&__GFP_HIGHMEM)
 {
  mflags&=(~VMDHA_USEHMA | VMDGA_16M);
  mflags|=VMDHA_USEHIGHMEM;
 }
*/
 p=__LX_malloc(size,mflags);
 if(!p)
 {
  if(gfp_mask&__GFP_REPEAT)
   p=__LX_malloc(size,mflags);
  else if(gfp_mask&__GFP_NOFAIL)
  {
   while(!p)
    p=__LX_malloc(size,mflags);
  }
 }
 return p;
}

//----------------------------------- LX__pa -----------------------------------
unsigned long LX__pa(volatile void* address)
{
 unsigned long f;
 struct lx_memobj* pm;
 unsigned long pa=0;
 unsigned long nrpages=5;
 PAGELIST pageList[5];
 spin_lock_irqsave(&memlist_lock,f);
 pm=LX_find_memobj((void*)address,0);
 if(pm)
 {
  if(!pm->physAddr)
  {
   if(!DevLinToPageList((LINEAR)pm->virtAddr,pm->size,(PAGELIST*)pageList))
    pm->physAddr=(void*)pageList[0].physaddr;
  }
  pa=(unsigned long)(pm->physAddr);
 }
 else
 { // Map unknown virtual address here
  pm=LX_alloc_memobj();
  if(pm)
  {
   pm->virtAddr=(void*)address;
   pm->unalignedAddr=(void*)address;
   pm->memflags=0;
   pm->size=PAGE_SIZE;
   pm->type=LXMT_VIRT_TO_PHYS;
   if(!DevLinToPageList((LINEAR)pm->virtAddr,pm->size,(PAGELIST*)pageList))
   {
    pm->physAddr=(void*)pageList[0].physaddr;
    pa=(unsigned long)pm->physAddr;
    list_add(&pm->list,&memobjs);
   }
   else
    LX_free(pm);
  }
 }
 spin_unlock_irqrestore(&memlist_lock,f);
 return pa;
}

//----------------------------------- LX__va -----------------------------------
void* LX__va(unsigned long address)
{
 unsigned long f;
 struct lx_memobj* pm;
 unsigned long va=0;
 spin_lock_irqsave(&memlist_lock,f);
 pm=LX_find_memobj((void*)address,1);
 if(pm)
  va=(unsigned long)pm->virtAddr;
 else
 {
  pm=LX_alloc_memobj();
  if(pm)
  {
   pm->physAddr=(void*)address;
   pm->unalignedAddr=(void*)address;
   pm->memflags=0;
   pm->size=4096;
   pm->type=LXMT_PHYS_TO_VIRT;
   if(DevVMAlloc(VMDHA_PHYS | VMDHA_PROCESS,pm->size
                 ,(LINEAR)&(pm->physAddr)
                 ,(LINEAR*)&(pm->virtAddr))
      && DevVMAlloc(VMDHA_PHYS | VMDHA_PROCESS | VMDHA_16M,pm->size
                    ,(LINEAR)&(pm->physAddr)
                    ,(LINEAR*)&(pm->virtAddr)))
   {
    DevVMFree((LINEAR)pm);
   }
   else
   {
    va=(unsigned long)pm->virtAddr;
    list_add(&pm->list,&memobjs);
   }
  }
 }
 spin_unlock_irqrestore(&memlist_lock,f);
 return (void*)va;
}

//------------------------------- LX_get_memstat -------------------------------
void LX_get_memstat(struct lx_memstat* memstat)
{
 unsigned long f;
 struct list_head* lh;
 struct lx_memobj* pm;
 memstat->total_sz=memstat->total_obj_sz=memstat->total_obj=memstat->total_sz_wasted=0;
 spin_lock_irqsave(&memlist_lock,f);
 list_for_each(lh,&memobjs)
 {
  pm=list_entry(lh,struct lx_memobj,list);
  memstat->total_sz+=sizeof(struct lx_memobj);
  memstat->total_obj_sz+=sizeof(struct lx_memobj);
  memstat->total_obj++;
  if(pm->type==LXMT_NORMAL_MALLOC)
  {
   memstat->total_sz+=pm->realsize;
   if(pm->size!=pm->realsize)
    memstat->total_sz_wasted+=pm->realsize;
  }
 }
 spin_unlock_irqrestore(&memlist_lock,f);
}

extern unsigned long lx_kernel_ds_size;
extern unsigned long lx_kernel_cs_size;
extern unsigned long lx_kernel_resident_ds_size;
extern unsigned long lx_kernel_resident_cs_size;
extern unsigned long lx_kernel_swappable_ds_size;
extern unsigned long lx_kernel_swappable_cs_size;
//------------------------------ LX_print_memstat ------------------------------
void LX_print_memstat(unsigned long flags)
{
 if(1==flags)
 {
  printk("Bytes in Resident Code Segments.: %d\n"
         ,lx_kernel_resident_cs_size+lx_kernel_cs_size);
  printk("Bytes in Resident Data Segments.: %d\n"
         ,lx_kernel_resident_ds_size+lx_kernel_ds_size);
  printk("Bytes in Swappable Code Segments: %d\n"
         ,lx_kernel_swappable_cs_size);
  printk("Bytes in Swappable Data Segments: %d\n"
         ,lx_kernel_swappable_ds_size);
 }
 else
 {
  struct lx_memstat memstat;
  LX_get_memstat(&memstat);
  printk("Number of memory objects........: %d\n",memstat.total_obj);
  printk("Allocated bytes.................: %d\n",memstat.total_sz);
  printk("Allocated bytes for obj usage...: %d\n",memstat.total_obj_sz);
  printk("Wasted alignment bytes..........: %d\n",memstat.total_sz_wasted);
 }
}

//------------------------- LX_wait_for_stable_memstat -------------------------
int LX_wait_for_stable_memstat(unsigned long flags)
{
 int i,c=0;
 struct lx_memstat memstat;
 unsigned long total_sz;
 if((flags&LX_WFSM_PRINTDRVSIZES))
 {
  LX_print_memstat(1);
  if((flags==LX_WFSM_PRINTDRVSIZES))
   return 0;
 }
 LX_get_memstat(&memstat);
 total_sz=memstat.total_sz;
 if(flags&LX_WFSM_PRINTNEWLINE)
  printk("\n");
 if(flags&LX_WFSM_PRINTSTATS_B)
  LX_print_memstat(0);
 if(flags&LX_WFSM_PRINTWAITMSG)
  printk("Waiting for stable memory statistics .");
 for(i=0;i<30;i++)
 {
  LX_call_shrinkers(10000);
  set_current_state(TASK_INTERRUPTIBLE);
  schedule_timeout(1000);
  set_current_state(TASK_RUNNING);
  if(flags&LX_WFSM_PRINTDOTS)
   printk(".");
  LX_get_memstat(&memstat);
  if(memstat.total_sz==total_sz)
   c++;
  else
   c=0;
  if(c>5)
  {
   if(flags&LX_WFSM_PRINTDOTS)
    printk("#");
   break;
  }
  total_sz=memstat.total_sz;
 }
 if(flags&LX_WFSM_PRINTWAITMSG)
  printk("\n");
 if(flags&LX_WFSM_PRINTSTATS_B)
  LX_print_memstat(0);
 if(memstat.total_sz==total_sz)
  return 1;
 else
  return 0;
}

//------------------------------ LX_alloc_objects ------------------------------
int LX_alloc_objects(int num_objects,int obj_size,unsigned long gfp_mask,...)
{
 va_list args;
 void* p=kmalloc(num_objects*obj_size,gfp_mask);
 if(!p)
  return -ENOMEM;
 va_start(args,gfp_mask);
 while(num_objects)
 {
  void** po=va_arg(args,void**);
  *po=p;
  p=(void*)(((unsigned long)p)+obj_size);
  num_objects--;
 }
 va_end(args);
 return 0;
}

extern int LX_ReInit_Stacks(unsigned long numIrqStacks);
//--------------------------- LX_InitComplete_Memory ---------------------------
void LX_InitComplete_Memory(void)
{
 unsigned long f;
 struct list_head* lh;
 struct list_head* lh2;
 LIST_HEAD(nl);
 struct lx_memobj* oldobj;
 struct lx_memobj* newobj;
 lx_default_malloc_flags&=~(VMDHA_USEHIGHMEM);
 lx_memobj_malloc_flags&=~(VMDHA_USEHIGHMEM);
// LX_ReInit_Stacks(CONFIG_LXIRQSTACKS);
 LX_ReInit_Stacks(4);
 spin_lock_irqsave(&memlist_lock,f);
 list_for_each_safe(lh,lh2,&memobjs)
 {
  oldobj=list_entry(lh,struct lx_memobj,list);
  newobj=LX_alloc_memobj();
  memcpy(newobj,oldobj,sizeof(struct lx_memobj));
  list_add(&newobj->list,&nl);
  list_del(&oldobj->list);
  DevVMFree((LINEAR)oldobj);
 }
 list_splice(&nl,&memobjs);
 spin_unlock_irqrestore(&memlist_lock,f);
}

//-------------------------------- LX_virtToLin --------------------------------
void* LX_virtToLin(void *addr)
{
 unsigned long rc;
 __asm__ __volatile__
 (
  "xorl  %%esi,%%esi;"
  "movw  %%ax,%%si;"
  "shrl  $16,%%eax;"
  "movb  $0x5b,%%dl;"
  "call  DevHlp;"
  "jnc   1f;"
  "xorl  %%eax,%%eax;"
  "1:;"
  :"=a"((long)rc)
  :"a"((long)addr)
  :"memory", "esi", "edx"
 );
 return (void*)rc;
}

//------------------------------- LX_verifyToLin -------------------------------
void* LX_verifyToLin(unsigned long addr,unsigned short length,unsigned char type)
{
 unsigned long rc;
 __asm__ __volatile__
 (
  "movb  %%dl,%%dh;"
  "xorl  %%edi,%%edi;"
  "movw  %%ax,%%di;"
  "shrl  $16,%%eax;"
  "pushl %%edi;"
  "pushl %%eax;"
  "movb  $0x27,%%dl;"
  "call  DevHlp;"
  "popl  %%eax;"
  "popl  %%esi;"
  "jc    1f;"
  "movb  $0x5b,%%dl;"
  "call  DevHlp;"
  "jnc   2f;"
  "1:;"
  "xorl  %%eax,%%eax;"
  "2:;"
  :"=a"((long)rc)
  :"a"((long)addr),"c"((short)length),"d"((char)type)
  :"memory", "ebx", "edi", "esi"
 );
 return (void*)rc;
};
