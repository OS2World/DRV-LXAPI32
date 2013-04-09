/* $Id: lk_resource.c,v 1.2 2004/07/23 21:42:30 smilcke Exp $ */

/*
 * resource.c
 * Autor:               Stefan Milcke
 * Erstellt am:         25.10.2001
 * Letzte Aenderung am: 21.07.2004
 *
*/

#include <lxcommon.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <asm/io.h>

struct resource ioport_resource = {
   .name = "PCI IO",
   .start   = 0x0000,
   .end  = IO_SPACE_LIMIT,
   .flags   = IORESOURCE_IO,
};

//EXPORT_SYMBOL(ioport_resource);

struct resource iomem_resource = {
   .name = "PCI mem",
   .start   = 0UL,
   .end  = ~0UL,
   .flags   = IORESOURCE_MEM,
};

//EXPORT_SYMBOL(iomem_resource);

static rwlock_t resource_lock=RW_LOCK_UNLOCKED;

//----------------------------- __request_resource -----------------------------
static struct resource *__request_resource(struct resource *root
                                           ,struct resource *newr)
{
 unsigned long start=newr->start;
 unsigned long end=newr->end;
 struct resource *tmp;
 struct resource **p;
 if(end<start)
  return root;
 if(start<root->start)
  return root;
 if(end>root->end)
  return root;
 p=&root->child;
 for(;;)
 {
  tmp=*p;
  if(!tmp||tmp->start>end)
  {
   newr->sibling=tmp;
   *p=newr;
   newr->parent=root;
   return 0;
  }
  p=&tmp->sibling;
  if(tmp->end<start)
   continue;
  return tmp;
 }
}

//----------------------------- __release_resource -----------------------------
static int __release_resource(struct resource *old)
{
 struct resource *tmp;
 struct resource **p;
 p=&old->parent->child;
 for(;;)
 {
  tmp=*p;
  if(!tmp)
   break;
  if(tmp==old)
  {
   *p=tmp->sibling;
   old->parent=0;
   return 0;
  }
  p=&tmp->sibling;
 }
 return -EINVAL;
}

//------------------------------ request_resource ------------------------------
int request_resource(struct resource *root,struct resource *newr)
{
 struct resource *conflict;
 write_lock(&resource_lock);
 conflict=__request_resource(root,newr);
 write_unlock(&resource_lock);
 return conflict ? -EBUSY : 0;
}

//------------------------------ release_resource ------------------------------
int release_resource(struct resource *old)
{
 int retval;
// write_lock((spinlock_t *)&resource_lock);
 retval=__release_resource(old);
// write_unlock((spinlock_t *)&resource_lock);
 return retval;
}

//------------------------------- find_resource --------------------------------
static int find_resource(struct resource *root,struct resource *newr
                         ,unsigned long size
                         ,unsigned long min,unsigned long max
                         ,unsigned long align
                         ,void (*alignf)(void *,struct resource *,
                                         unsigned long,unsigned long)
                         ,void *alignf_data)
{
 struct resource *thisr=root->child;
 newr->start=root->start;
 for(;;)
 {
  if(thisr)
   newr->end=thisr->start;
  else
   newr->end=root->end;
  if(newr->start<min)
   newr->start=min;
  if(newr->end>max)
   newr->end=max;
  newr->start=(newr->start + align-1)&~(align-1);
  if(alignf)
   alignf(alignf_data,newr,size,align);
  if(newr->start<newr->end && newr->end-newr->start+1>=size)
  {
   newr->end=newr->start+size-1;
   return 0;
  }
  if(!thisr)
   break;
  newr->start=thisr->end+1;
  thisr=thisr->sibling;
 }
 return -EBUSY;
}

//----------------------------- allocate_resource ------------------------------
int allocate_resource(struct resource *root,struct resource *newr
                      ,unsigned long size
                      ,unsigned long min,unsigned long max
                      ,unsigned long align
                      ,void (*alignf)(void *,struct resource*,
                                      unsigned long,unsigned long)
                      ,void *alignf_data)
{
 int err;
 write_lock(&resource_lock);
 err=find_resource(root,newr,size,min,max,align,alignf,alignf_data);
 if(err>=0 && __request_resource(root,newr))
  err=-EBUSY;
 write_unlock(&resource_lock);
 return err;
}
