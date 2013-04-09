/* $Id: lxmemory.h,v 1.10 2005/07/12 21:59:37 smilcke Exp $ */

/*
 * lxmemory.h
 * Autor:               Stefan Milcke
 * Erstellt am:         16.01.2005
 * Letzte Aenderung am: 12.07.2005
 *
*/

#ifndef LXMEMORY_H_INCLUDED
#define LXMEMORY_H_INCLUDED

struct lx_memstat
{
 unsigned long total_obj;
 unsigned long total_sz;
 unsigned long total_obj_sz;
 unsigned long total_sz_wasted;
};

/* Memory management */
extern unsigned long lx_default_malloc_flags;
extern unsigned long lx_memobj_malloc_flags;

#ifdef LXMALLOC_DECLARATIONS
extern unsigned long lx_default_malloc_flags;
extern unsigned long lx_memobj_malloc_flags;
extern unsigned short LX_allocGDTSelector(void);
extern int LX_freeGDTSelector(unsigned short selector);
extern asmlinkage
 unsigned long LXA_VMAllocAligned(unsigned long flags
                                  ,unsigned long size,unsigned long* aligned_mem
                                  ,unsigned long* real_size,unsigned long* real_mem);

extern void* __LX_malloc(unsigned long size,unsigned long flags);
extern void  __LX_free(void* ptr);
extern void* __LX_malloc_gfp(unsigned long size,unsigned long gfp_mask);

#define LX_malloc(size) __LX_malloc(size,lx_default_malloc_flags)
#define LX_free(ptr) __LX_free(ptr)
#define LX_malloc_gfp(size,gfp_mask) __LX_malloc_gfp(size,gfp_mask)
#define LX_free_gfp(ptr) __LX_free(ptr)

#endif  // LXMALLOC_DECLARATIONS

/* Multiple object allocation */
extern int LX_alloc_objects(int num_objects,int obj_size,unsigned long gfp_mask,...);

/* Memory information */
extern void LX_print_memstat(unsigned long flags);
extern void LX_get_memstat(struct lx_memstat* memstat);

/* shrinkers */
extern int LX_call_shrinkers(int count);

/* Wrappers */
struct page* fastcall LX_alloc_num_pages(unsigned int gfp_mask,unsigned int num_pages);


#define LX_WFSM_PRINTWAITMSG  (1)
#define LX_WFSM_PRINTDOTS     (2)
#define LX_WFSM_PRINTNEWLINE  (4)
#define LX_WFSM_PRINTSTATS_B  (8)
#define LX_WFSM_PRINTSTATS_A  (16)
#define LX_WFSM_PRINTDRVSIZES (32)
#define LX_WFSM_PRINTALLW     (LX_WFSM_PRINTWAITMSG|LX_WFSM_PRINTDOTS|LX_WFSM_PRINTNEWLINE)
#define LX_WFSM_PRINTALL      (LX_WFSM_PRINTALLW|LX_WFSM_PRINTSTATS_B|LX_WFSM_PRINTSTATS_A)
extern int LX_wait_for_stable_memstat(unsigned long flags);

#endif //LXMEMORY_H_INCLUDED
