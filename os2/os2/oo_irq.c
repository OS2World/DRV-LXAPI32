/* $Id: oo_irq.c,v 1.4 2005/02/06 22:49:34 smilcke Exp $ */

/*
 * oo_irq.c
 * Autor:               Stefan Milcke
 * Erstellt am:         10.11.2001
 * Letzte Aenderung am: 01.02.2005
 *
*/

#include <lxcommon.h>
#include "irqos2.h"
#include <lxrmcall.h>

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/timer.h>
//#include <linux/irq.h>
#include <asm/semaphore.h>
#include <linux/signal.h>
#include <linux/init.h>

#include <lxapi.h>
#include <lxapioctl.h>

extern unsigned long LX_request_irq(struct lxrm_resource* rm_resource
                                    ,unsigned int irq,unsigned long flags);
extern unsigned long LX_free_irq(struct lxrm_resource* rm_resource
                                 ,unsigned int irq);
extern asmlinkage unsigned int do_IRQ(struct pt_regs regs);

#define IRQ_STATE_CLAIMED  (0x10000)
#define IRQ_STATE_EOI      (0x20000)

//-------------------------------- LX_IRQEntry ---------------------------------
__attribute__((regparm(3)))
unsigned long LX_IRQEntry(int irq)
{
#ifdef LXDEBUG
 static int irq_depth=0;
 if(irq_depth)
  return 0;
#endif
 jiffies_64=(u64)lx_InfoSegGDTPtr->SIS_MsCount;
 jiffies=(unsigned long)jiffies_64;
#ifdef LXDEBUG
 irq_depth++;
#endif
 if(irq<=15)
 {
  unsigned long ToDoAfterIrq=irq;
  struct pt_regs regs={0};
  static int u=0;
  regs.orig_eax=irq;
  LX_enter_irq_current();
  atomic_inc(&lx_in_ISR);

  // Set flags to fake user mode and kernel mode
  u++;
  if(u<1)
  {
   regs.xcs&=~3;
   regs.eflags&=~VM_MASK;
  }
  else
  {
   regs.xcs|=3;
   regs.eflags|=VM_MASK;
   u=0;
  }
  do_IRQ(regs);
  atomic_dec(&lx_in_ISR);
  if(LX_irq_handled)
  {
   ToDoAfterIrq|=IRQ_STATE_CLAIMED;
   ToDoAfterIrq|=IRQ_STATE_EOI;
  }
  LX_leave_irq_current();
#ifdef LXDEBUG
  irq_depth--;
  lx_call_depth=0;
#endif
  return ToDoAfterIrq;
 }
 else
 {
#ifdef LXDEBUG
  irq_depth--;
  lx_call_depth=0;
#endif
  return 0;
 }
}

