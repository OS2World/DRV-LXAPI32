/* $Id: lak_smpboot.c,v 1.3 2004/07/23 21:42:20 smilcke Exp $ */

/*
 * lak_smpboot.c
 * Autor:               Stefan Milcke
 * Erstellt am:         30.06.2004
 * Letzte Aenderung am: 21.07.2004
 *
*/

#include <lxcommon.h>

/*
 * x86 SMP booting functions
 *
 * (c) 1995 Alan Cox, Building #3 <alan@redhat.com>
 * (c) 1998, 1999, 2000 Ingo Molnar <mingo@redhat.com>
 *
 * Much of the core SMP work is based on previous work by Thomas Radke, to
 * whom a great many thanks are extended.
 *
 * Thanks to Intel for making available several different Pentium,
 * Pentium Pro and Pentium-II/Xeon MP machines.
 * Original development of Linux SMP code supported by Caldera.
 *
 * This code is released under the GNU General Public License version 2 or
 * later.
 *
 * Fixes
 *    Felix Koop  :  NR_CPUS used properly
 *    Jose Renau  :  Handle single CPU case.
 *    Alan Cox :  By repeated request 8) - Total BogoMIP report.
 *    Greg Wright :  Fix for kernel stacks panic.
 *    Erich Boleyn   :  MP v1.4 and additional changes.
 * Matthias Sattler  :  Changes for 2.1 kernel map.
 * Michel Lespinasse :  Changes for 2.1 kernel map.
 * Michael Chastain  :  Change trampoline.S to gnu as.
 *    Alan Cox :  Dumb bug: 'B' step PPro's are fine
 *    Ingo Molnar :  Added APIC timers, based on code
 *             from Jose Renau
 *    Ingo Molnar :  various cleanups and rewrites
 *    Tigran Aivazian   :  fixed "0.00 in /proc/uptime on SMP" bug.
 * Maciej W. Rozycki :  Bits for genuine 82489DX APICs
 *    Martin J. Bligh   :  Added support for multi-quad systems
 *    Dave Jones  :  Report invalid combinations of Athlon CPUs.
*     Rusty Russell  :  Hacked into shape for new "hotplug" boot process. */

#include <linux/module.h>
#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/mm.h>
#include <linux/kernel_stat.h>
#include <linux/smp_lock.h>
#include <linux/irq.h>
#include <linux/bootmem.h>

#include <linux/delay.h>
#include <linux/mc146818rtc.h>
#include <asm/pgalloc.h>
#include <asm/tlbflush.h>
#include <asm/desc.h>
#include <asm/arch_hooks.h>

#include <mach_apic.h>
#include <mach_wakecpu.h>
#include <smpboot_hooks.h>

/* Per CPU bogomips and other parameters */
struct cpuinfo_x86 cpu_data[NR_CPUS] __cacheline_aligned;
int phys_proc_id[NR_CPUS];

//---------------------------------- __cpu_up ----------------------------------
int __devinit __cpu_up(unsigned int cpu)
{
 return 0;
}
