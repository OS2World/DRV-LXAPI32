/* $Id: lakcm_k7.c,v 1.2 2004/10/31 23:47:04 smilcke Exp $ */

/*
 * lakcm_k7.c
 * Autor:               Stefan Milcke
 * Erstellt am:         21.07.2004
 * Letzte Aenderung am: 07.10.2004
 *
*/

#include <lxcommon.h>

/*
 * Athlon/Hammer specific Machine Check Exception Reporting
 * (C) Copyright 2002 Dave Jones <davej@codemonkey.org.uk>
 */

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/smp.h>

#include <asm/processor.h>
#include <asm/system.h>
#include <asm/msr.h>

#include "mce.h"

/* Machine Check Handler For AMD Athlon/Duron */
static asmlinkage void k7_machine_check(struct pt_regs * regs, long error_code)
{
   int recover=1;
   u32 alow, ahigh, high, low;
   u32 mcgstl, mcgsth;
   int i;

   rdmsr (MSR_IA32_MCG_STATUS, mcgstl, mcgsth);
   if (mcgstl & (1<<0)) /* Recoverable ? */
      recover=0;

   printk (KERN_EMERG "CPU %d: Machine Check Exception: %08x%08x\n",
      smp_processor_id(), mcgsth, mcgstl);

   for (i=1; i<nr_mce_banks; i++) {
      rdmsr (MSR_IA32_MC0_STATUS+i*4,low, high);
      if (high&(1<<31)) {
         if (high & (1<<29))
            recover |= 1;
         if (high & (1<<25))
            recover |= 2;
         printk (KERN_EMERG "Bank %d: %08x%08x", i, high, low);
         high &= ~(1<<31);
         if (high & (1<<27)) {
            rdmsr (MSR_IA32_MC0_MISC+i*4, alow, ahigh);
            printk ("[%08x%08x]", ahigh, alow);
         }
         if (high & (1<<26)) {
            rdmsr (MSR_IA32_MC0_ADDR+i*4, alow, ahigh);
            printk (" at %08x%08x", ahigh, alow);
         }
         printk ("\n");
         /* Clear it */
         wrmsr (MSR_IA32_MC0_STATUS+i*4, 0UL, 0UL);
         /* Serialize */
         wmb();
      }
   }

   if (recover&2)
      panic ("CPU context corrupt");
   if (recover&1)
      panic ("Unable to continue");
   printk (KERN_EMERG "Attempting to continue.\n");
   mcgstl &= ~(1<<2);
   wrmsr (MSR_IA32_MCG_STATUS,mcgstl, mcgsth);
}


/* AMD K7 machine check is Intel like */
void __init amd_mcheck_init(struct cpuinfo_x86 *c)
{
   u32 l, h;
   int i;

   machine_check_vector = k7_machine_check;
   wmb();

   printk (KERN_INFO "Intel machine check architecture supported.\n");
   rdmsr (MSR_IA32_MCG_CAP, l, h);
   if (l & (1<<8))   /* Control register present ? */
      wrmsr (MSR_IA32_MCG_CTL, 0xffffffff, 0xffffffff);
   nr_mce_banks = l & 0xff;

   /* Clear status for MC index 0 separately, we don't touch CTL,
    * as some Athlons cause spurious MCEs when its enabled. */
   wrmsr (MSR_IA32_MC0_STATUS, 0x0, 0x0);
   for (i=1; i<nr_mce_banks; i++) {
      wrmsr (MSR_IA32_MC0_CTL+4*i, 0xffffffff, 0xffffffff);
      wrmsr (MSR_IA32_MC0_STATUS+4*i, 0x0, 0x0);
   }

   set_in_cr4 (X86_CR4_MCE);
   printk (KERN_INFO "Intel machine check reporting enabled on CPU#%d.\n",
      smp_processor_id());
}
