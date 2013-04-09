/* $Id: lxsegments.c,v 1.6 2006/04/27 21:17:52 smilcke Exp $ */

/*
 * lxsegments.c
 * Autor:               Stefan Milcke
 * Erstellt am:         23.02.2005
 * Letzte Aenderung am: 25.04.2006
 *
*/

#define LXMALLOC_DECLARATIONS
#include <lxcommon.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>

#include <lxapi.h>

#define MAX_SEGMENT_SIZE   (65535)

//------------------------------ LX_alloc_segment ------------------------------
struct lx_segment* LX_alloc_segment(unsigned long type,unsigned long size
                                    ,unsigned long virt_addr
                                    ,struct lx_segment* segptr)
{
 int i;
 unsigned long lastphysical;
 struct lx_segment* segment;
 if(!segptr)
  segment=kmalloc(sizeof(struct lx_segment),GFP_KERNEL);
 else
  segment=segptr;
 if(!segment)
  return 0;
 memset(segment,0,sizeof(struct lx_segment));
 segment->type=type;
 segment->size=size;
 if(!size)
  return segment;
 if(0==virt_addr)
  segment->pVirtual=__LX_malloc(size,0);
 else
  segment->pVirtual=(void*)virt_addr;
 if(!segment->pVirtual)
  goto no_virtual;
 segment->pPhysical=(void*)LX__pa(segment->pVirtual);
 if(!segment->pPhysical)
  goto no_physical;
 segment->numSels=size/MAX_SEGMENT_SIZE+1;
 segment->sels=kmalloc(sizeof(unsigned short)*segment->numSels,GFP_KERNEL);
 if(!segment->sels)
  goto no_selarray;
 memset(segment->sels,0,sizeof(unsigned short)*segment->numSels);
 lastphysical=(unsigned long)segment->pPhysical;
 for(i=0;i<segment->numSels;i++)
 {
  unsigned long sz=size>MAX_SEGMENT_SIZE ? MAX_SEGMENT_SIZE : size;
  segment->sels[i]=LX_allocGDTSelector();
  if(!segment->sels[i])
   goto no_sel;
  if(DevPhysToGDTSel(lastphysical,sz,segment->sels[i],segment->type))
   goto no_map;
  if(!segment->pMapped)
   if(DevVirtToLin(segment->sels[i],0,(LINEAR*)&segment->pMapped))
    goto no_lin;
  lastphysical+=sz;
  size-=sz;
 }
 return segment;
no_lin:
no_map:
no_sel:
 for(i=0;i<segment->numSels;i++)
 {
  if(segment->sels[i])
   LX_freeGDTSelector(segment->sels[i]);
 }
no_selarray:
no_physical:
no_virtual:
 kfree(segment);
 return 0;
}

//------------------------------ LX_free_segment -------------------------------
unsigned long LX_free_segment(struct lx_segment* segment)
{
 if(segment)
 {
  int i;
  for(i=0;i<segment->numSels;i++)
  {
   if(segment->sels[i])
    LX_freeGDTSelector(segment->sels[i]);
  }
  if(segment->pVirtual)
   __LX_free(segment->pVirtual);
  kfree(segment);
 }
 return 0;
}

extern unsigned long _code_segments;
extern unsigned long _data_segments;
extern unsigned long _SEGSTRT__init_text;
extern unsigned long _SEGSTRT__init_data;
extern unsigned long _SEGSTRT__init_setup;
extern unsigned long _SEGSTRT__initcall1_init;
extern unsigned long _SEGSTRT__initcall2_init;
extern unsigned long _SEGSTRT__initcall3_init;
extern unsigned long _SEGSTRT__initcall4_init;
extern unsigned long _SEGSTRT__initcall5_init;
extern unsigned long _SEGSTRT__initcall6_init;
extern unsigned long _SEGSTRT__initcall7_init;
extern unsigned long _SEGEND__init_text;
extern unsigned long _SEGEND__init_data;
extern unsigned long _SEGEND__init_setup;
extern unsigned long _SEGEND__initcall1_init;
extern unsigned long _SEGEND__initcall2_init;
extern unsigned long _SEGEND__initcall3_init;
extern unsigned long _SEGEND__initcall4_init;
extern unsigned long _SEGEND__initcall5_init;
extern unsigned long _SEGEND__initcall6_init;
extern unsigned long _SEGEND__initcall7_init;
extern unsigned long lx_kernel_resident_cs_size;
extern unsigned long lx_kernel_resident_ds_size;
extern unsigned long lx_kernel_swappable_cs_size;
extern unsigned long lx_kernel_swappable_ds_size;

//--------------------------- __LX_lock_gcc_segment ----------------------------
static unsigned long __LX_lock_gcc_segment(unsigned long start,unsigned long end
                                            ,unsigned long is_data)
{
 if(end-start>0)
 {
  if(is_data)
   lx_kernel_resident_ds_size+=(end-start);
  else
   lx_kernel_resident_cs_size+=(end-start);
  return LX_lock_mem(start,end-start
                     ,is_data ? VMDHL_LONG | VMDHL_WRITE : VMDHL_LONG);
 }
 return 0;
}

//--------------------------- __LX_lock_gcc_segments ---------------------------
static unsigned long __LX_lock_gcc_segments(unsigned long* p
                                             ,unsigned long is_data)
{
 unsigned long rc=0;
 unsigned long s,e;
 while(*p)
 {
  s=*p++;
  e=*p++;
  rc=__LX_lock_gcc_segment(s,e,is_data);
  if(rc)
   break;
 }
 return rc;
}

//---------------------------- LX_lock_gcc_segments ----------------------------
unsigned long LX_lock_gcc_segments(void)
{
 unsigned long rc;
 rc=__LX_lock_gcc_segments(&_code_segments,0);
 if(!rc)
  rc=__LX_lock_gcc_segments(&_data_segments,1);
 return rc;
}

//---------------------------- __LX_unlock_segment -----------------------------
static unsigned long __LX_unlock_segment(unsigned long start,unsigned long* p
                                         ,unsigned long is_data)
{
 unsigned long rc=-ENOENT;
 unsigned long s,e;
 while(*p)
 {
  s=*p++;
  e=*p++;
  if(s==start)
  {
   if(e-s>0)
   {
    unsigned long sz=e-s;
    rc=LX_unlock_mem(start);
    if(is_data)
    {
     lx_kernel_resident_ds_size-=sz;
     lx_kernel_swappable_ds_size+=sz;
    }
    else
    {
     lx_kernel_resident_cs_size-=sz;
     lx_kernel_swappable_cs_size+=sz;
    }
   }
   break;
  }
 }
 return rc;
}

//----------------------------- LX_unlock_segment ------------------------------
unsigned long LX_unlock_segment(unsigned long start)
{
 if(__LX_unlock_segment(start,&_code_segments,0))
  __LX_unlock_segment(start,&_data_segments,1);
 return 0;
}

//-------------------------- LX_unlock_init_segments ---------------------------
unsigned long LX_unlock_init_segments(void)
{
 LX_unlock_segment((unsigned long)&_SEGSTRT__init_data);
 LX_unlock_segment((unsigned long)&_SEGSTRT__init_setup);
 LX_unlock_segment((unsigned long)&_SEGSTRT__initcall1_init);
 LX_unlock_segment((unsigned long)&_SEGSTRT__initcall2_init);
 LX_unlock_segment((unsigned long)&_SEGSTRT__initcall3_init);
 LX_unlock_segment((unsigned long)&_SEGSTRT__initcall4_init);
 LX_unlock_segment((unsigned long)&_SEGSTRT__initcall5_init);
 LX_unlock_segment((unsigned long)&_SEGSTRT__initcall6_init);
 LX_unlock_segment((unsigned long)&_SEGSTRT__initcall7_init);
 LX_unlock_segment((unsigned long)&_SEGSTRT__init_text);
 return 0;
}
