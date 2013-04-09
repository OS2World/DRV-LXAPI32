/* $Id: oak_vm86.c,v 1.1 2004/12/16 23:03:25 smilcke Exp $ */

/*
 * oak_vm86.c
 * Autor:               Stefan Milcke
 * Erstellt am:         15.11.2004
 * Letzte Aenderung am: 15.11.2004
 *
*/

#include <lxcommon.h>
#include <linux/kernel.h>

//------------------------------ release_x86_irqs ------------------------------
void release_x86_irqs(struct task_struct* task)
{
#ifndef TARGET_OS2
 int i;
 for(i=FIRST_VM86_IRQ;i<=LAST_VM86_IRQ;i++)
  if(vm86_irqs[i].tsk==task)
   free_vm86_irq(i);
#endif
}
