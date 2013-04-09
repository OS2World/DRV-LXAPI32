/* $Id: ol_ioremap.c,v 1.2 2005/03/22 22:34:40 smilcke Exp $ */

/*
 * ioremap.c
 * Autor:               Stefan Milcke
 * Erstellt am:         08.11.2001
 * Letzte Aenderung am: 25.02.2005
 *
*/

#define LXMALLOC_DECLARATIONS
#include <lxcommon.h>

struct mapped_mem
{
 SEL sel;
 unsigned long linaddr;
 unsigned short pid;
 unsigned short tid;
};

struct mapped_mem mmem[MAX_GDTSELECTORS]={{0}};

//---------------------------------- ioremap -----------------------------------
unsigned long ioremap(unsigned long addr,unsigned short usSize)
{
 unsigned long linaddr=0;
 SEL sel=(SEL)LX_allocGDTSelector();
 if(!DevPhysToGDTSelector((PHYSICAL)addr,(WORD16)usSize,sel))
 {
  if(DevVirtToLin(sel
                  ,0
                  ,(LINEAR*)&linaddr))
   linaddr=0;
 }
 if(!linaddr)
  LX_freeGDTSelector(sel);
 else
 {
  int i;
  for(i=0;i<MAX_GDTSELECTORS;i++)
  {
   if(0==(mmem[i].sel))
   {
    mmem[i].sel=sel;
    mmem[i].linaddr=linaddr;
    mmem[i].pid=lx_current_pid;
    mmem[i].tid=lx_current_tid;
    break;
   }
  }
 }
 return linaddr;
}

//---------------------------------- iounmap -----------------------------------
void iounmap(void *addr)
{
 int i;
 for(i=0;i<MAX_GDTSELECTORS;i++)
 {
  if(0!=mmem[i].sel && mmem[i].linaddr==(unsigned long)addr)
  {
   LX_freeGDTSelector(mmem[i].sel);
   mmem[i].sel=0;
   mmem[i].linaddr=0;
   break;
  }
 }
}
