/* $Id: lakc_nexgen.c,v 1.2 2004/10/31 23:47:03 smilcke Exp $ */

/*
 * lakc_nexgen.c
 * Autor:               Stefan Milcke
 * Erstellt am:         21.07.2004
 * Letzte Aenderung am: 07.10.2004
 *
*/

#include <lxcommon.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <asm/processor.h>

#include "cpu.h"

/*
 * Detect a NexGen CPU running without BIOS hypercode new enough
 * to have CPUID. (Thanks to Herbert Oppmann)
 */

static int __init deep_magic_nexgen_probe(void)
{
   int ret;

   __asm__ __volatile__ (
      "  movw  $0x5555, %%ax\n"
      "  xorw  %%dx,%%dx\n"
      "  movw  $2, %%cx\n"
      "  divw  %%cx\n"
      "  movl  $0, %%eax\n"
      "  jnz   1f\n"
      "  movl  $1, %%eax\n"
      "1:\n"
      : "=a" (ret) : : "cx", "dx" );
   return  ret;
}

static void __init init_nexgen(struct cpuinfo_x86 * c)
{
   c->x86_cache_size = 256; /* A few had 1 MB... */
}

static void nexgen_identify(struct cpuinfo_x86 * c)
{
   /* Detect NexGen with old hypercode */
   if ( deep_magic_nexgen_probe() ) {
      strcpy(c->x86_vendor_id, "NexGenDriven");
   }
   generic_identify(c);
}

static struct cpu_dev nexgen_cpu_dev __initdata = {
   .c_vendor   = "Nexgen",
   .c_ident = { "NexGenDriven" },
   .c_models = {
         { .vendor = X86_VENDOR_NEXGEN,
           .family = 5,
           .model_names = { [1] = "Nx586" }
         },
   },
   .c_init     = init_nexgen,
   .c_identify = nexgen_identify,
};

int __init nexgen_init_cpu(void)
{
   cpu_devs[X86_VENDOR_NEXGEN] = &nexgen_cpu_dev;
   return 0;
}

//early_arch_initcall(nexgen_init_cpu);
