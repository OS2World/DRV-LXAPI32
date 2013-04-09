/* $Id: oak_io_apic.c,v 1.1 2004/09/29 23:27:33 smilcke Exp $ */

/*
 * oak_io_apic.c
 * Autor:               Stefan Milcke
 * Erstellt am:         21.09.2004
 * Letzte Aenderung am: 21.09.2004
 *
*/

#include <lxcommon.h>
/*
 * Intel IO-APIC support for multi-Pentium hosts.
 *
 * Copyright (C) 1997, 1998, 1999, 2000 Ingo Molnar, Hajnalka Szabo
 *
 * Many thanks to Stig Venaas for trying out countless experimental
 * patches and reporting/debugging problems patiently!
 *
 * (c) 1999, Multiple IO-APIC support, developed by
 * Ken-ichi Yaku <yaku@css1.kbnes.nec.co.jp> and
 *      Hidemi Kishimoto <kisimoto@css1.kbnes.nec.co.jp>,
 * further tested and cleaned up by Zach Brown <zab@redhat.com>
 * and Ingo Molnar <mingo@redhat.com>
 *
 * Fixes
 * Maciej W. Rozycki :  Bits for genuine 82489DX APICs;
 *             thanks to Eric Gilmore
 *             and Rolf G. Tews
 *             for testing these extensively
 * Paul Diefenbaugh  :  Added full ACPI support
 */

#include <linux/mm.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/config.h>
#include <linux/smp_lock.h>
#include <linux/mc146818rtc.h>
#include <linux/compiler.h>
//#include <linux/acpi.h>

#include <asm/io.h>
#include <asm/smp.h>
#include <asm/desc.h>
#include <asm/timer.h>

/* irq_vectors is indexed by the sum of all RTEs in all I/O APICs. */
u8 irq_vector[NR_IRQ_VECTORS] = { FIRST_DEVICE_VECTOR , 0 };
