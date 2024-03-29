/* $Id: lakc_transmeta.c,v 1.2 2004/10/31 23:47:03 smilcke Exp $ */

/*
 * lakc_transmeta.c
 * Autor:               Stefan Milcke
 * Erstellt am:         21.07.2004
 * Letzte Aenderung am: 07.10.2004
 *
*/

#include <lxcommon.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/processor.h>
#include <asm/msr.h>
#include "cpu.h"

static void __init init_transmeta(struct cpuinfo_x86 *c)
{
   unsigned int cap_mask, uk, max, dummy;
   unsigned int cms_rev1, cms_rev2;
   unsigned int cpu_rev, cpu_freq, cpu_flags;
   char cpu_info[65];

   get_model_name(c);   /* Same as AMD/Cyrix */
   display_cacheinfo(c);

   /* Print CMS and CPU revision */
   max = cpuid_eax(0x80860000);
   if ( max >= 0x80860001 ) {
      cpuid(0x80860001, &dummy, &cpu_rev, &cpu_freq, &cpu_flags);
      printk(KERN_INFO "CPU: Processor revision %u.%u.%u.%u, %u MHz\n",
             (cpu_rev >> 24) & 0xff,
             (cpu_rev >> 16) & 0xff,
             (cpu_rev >> 8) & 0xff,
             cpu_rev & 0xff,
             cpu_freq);
   }
   if ( max >= 0x80860002 ) {
      cpuid(0x80860002, &dummy, &cms_rev1, &cms_rev2, &dummy);
      printk(KERN_INFO "CPU: Code Morphing Software revision %u.%u.%u-%u-%u\n",
             (cms_rev1 >> 24) & 0xff,
             (cms_rev1 >> 16) & 0xff,
             (cms_rev1 >> 8) & 0xff,
             cms_rev1 & 0xff,
             cms_rev2);
   }
   if ( max >= 0x80860006 ) {
      cpuid(0x80860003,
            (void *)&cpu_info[0],
            (void *)&cpu_info[4],
            (void *)&cpu_info[8],
            (void *)&cpu_info[12]);
      cpuid(0x80860004,
            (void *)&cpu_info[16],
            (void *)&cpu_info[20],
            (void *)&cpu_info[24],
            (void *)&cpu_info[28]);
      cpuid(0x80860005,
            (void *)&cpu_info[32],
            (void *)&cpu_info[36],
            (void *)&cpu_info[40],
            (void *)&cpu_info[44]);
      cpuid(0x80860006,
            (void *)&cpu_info[48],
            (void *)&cpu_info[52],
            (void *)&cpu_info[56],
            (void *)&cpu_info[60]);
      cpu_info[64] = '\0';
      printk(KERN_INFO "CPU: %s\n", cpu_info);
   }

   /* Unhide possibly hidden capability flags */
   rdmsr(0x80860004, cap_mask, uk);
   wrmsr(0x80860004, ~0, uk);
   c->x86_capability[0] = cpuid_edx(0x00000001);
   wrmsr(0x80860004, cap_mask, uk);

   /* If we can run i686 user-space code, call us an i686 */
#define USER686 (X86_FEATURE_TSC|X86_FEATURE_CX8|X86_FEATURE_CMOV)
        if ( c->x86 == 5 && (c->x86_capability[0] & USER686) == USER686 )
      c->x86 = 6;
}

static void transmeta_identify(struct cpuinfo_x86 * c)
{
   u32 xlvl;
   generic_identify(c);

   /* Transmeta-defined flags: level 0x80860001 */
   xlvl = cpuid_eax(0x80860000);
   if ( (xlvl & 0xffff0000) == 0x80860000 ) {
      if (  xlvl >= 0x80860001 )
         c->x86_capability[2] = cpuid_edx(0x80860001);
   }
}

static struct cpu_dev transmeta_cpu_dev __initdata = {
   .c_vendor   = "Transmeta",
   .c_ident = { "GenuineTMx86", "TransmetaCPU" },
   .c_init     = init_transmeta,
   .c_identify = transmeta_identify,
};

int __init transmeta_init_cpu(void)
{
   cpu_devs[X86_VENDOR_TRANSMETA] = &transmeta_cpu_dev;
   return 0;
}

//early_arch_initcall(transmeta_init_cpu);
