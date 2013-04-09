/* $Id: om_bootmem.c,v 1.7 2005/06/05 22:51:58 smilcke Exp $ */

/*
 * lx_bootmem.c
 * Autor:               Stefan Milcke
 * Erstellt am:         31.07.2004
 * Letzte Aenderung am: 05.06.2005
 *
*/

#include <lxcommon.h>
#include <linux/init.h>
#include <linux/slab.h>

//#define BOOTMEM_FLAGS (VMDHA_FIXED | VMDHA_USEHMA)
#define BOOTMEM_FLAGS   (VMDHA_USEHMA)

//----------------------------- init_bootmem_core ------------------------------
static unsigned long __init init_bootmem_core(pg_data_t *pgdat
   ,unsigned long mapstart,unsigned long start,unsigned long end)
{
 return 0;
}

#ifndef CONFIG_DISCONTIGMEM
//-------------------------------- init_bootmem --------------------------------
unsigned long __init init_bootmem(unsigned long start,unsigned long pages)
{
 // max_low_pfn=pages;
 // min_low_pfn=start;
 return(init_bootmem_core(&contig_page_data,start,0,pages));
}
#endif

//------------------------------ __alloc_bootmem -------------------------------
void* __init __alloc_bootmem(unsigned long size,unsigned long align,unsigned long goal)
{
 void* p;
 if(DevVMAlloc(BOOTMEM_FLAGS,size,(LINEAR)-1,(LINEAR*)&p))
  return NULL;
 return p;
}

//-------------------------------- free_bootmem --------------------------------
void __init free_bootmem(unsigned long addr,unsigned long size)
{
 DevVMFree((LINEAR)addr);
}

//------------------------------ reserve_bootmem -------------------------------
void __init reserve_bootmem(unsigned long addr,unsigned long size)
{
 NOT_IMPLEMENTED();
}

//------------------------------ free_all_bootmem ------------------------------
unsigned long __init free_all_bootmem(void)
{
 NOT_IMPLEMENTED();
 return 0;
}
