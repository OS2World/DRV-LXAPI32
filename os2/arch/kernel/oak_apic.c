/* $Id: oak_apic.c,v 1.1 2004/09/29 23:27:32 smilcke Exp $ */

/*
 * oak_apic.c
 * Autor:               Stefan Milcke
 * Erstellt am:         21.09.2004
 * Letzte Aenderung am: 22.09.2004
 *
*/

#include <lxcommon.h>
/*
 * Local APIC handling, local APIC timers
 *
 * (c) 1999, 2000 Ingo Molnar <mingo@redhat.com>
 *
 * Fixes
 * Maciej W. Rozycki :  Bits for genuine 82489DX APICs;
 *             thanks to Eric Gilmore
 *             and Rolf G. Tews
 *             for testing these extensively.
 * Maciej W. Rozycki :  Various updates and fixes.
 * Mikael Pettersson :  Power Management for UP-APIC.
 * Pavel Machek and
 * Mikael Pettersson :  PM converted to driver model.
 */

#include <linux/config.h>
#include <linux/init.h>

#include <linux/mm.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/bootmem.h>
#include <linux/smp_lock.h>
#include <linux/interrupt.h>
#include <linux/mc146818rtc.h>
#include <linux/kernel_stat.h>
#include <linux/sysdev.h>

#include <asm/atomic.h>
#include <asm/smp.h>
#include <asm/mtrr.h>
#include <asm/mpspec.h>
#include <asm/pgalloc.h>
#include <asm/desc.h>
#include <asm/arch_hooks.h>
#include <asm/hpet.h>

//------------------------------- apic_intr_init -------------------------------
void __init apic_intr_init(void)
{
}

//----------------------------- disable_local_APIC -----------------------------
void disable_local_APIC(void)
{
 NOT_IMPLEMENTED();
}

//------------------------------- init_bsp_APIC --------------------------------
void __init init_bsp_APIC(void)
{
}
