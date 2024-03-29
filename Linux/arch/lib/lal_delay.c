/* $Id: lal_delay.c,v 1.1 2004/07/23 21:42:25 smilcke Exp $ */

/*
 * lal_delay.c
 * Autor:               Stefan Milcke
 * Erstellt am:         21.07.2004
 * Letzte Aenderung am: 21.07.2004
 *
*/

#include <lxcommon.h>

/*
 * Precise Delay Loops for i386
 *
 * Copyright (C) 1993 Linus Torvalds
 * Copyright (C) 1997 Martin Mares <mj@atrey.karlin.mff.cuni.cz>
 *
 * The __delay function must _NOT_ be inlined as its execution time
 * depends wildly on alignment on many x86 processors. The additional
 * jump magic is needed to get the timing stable on all the CPU's
 * we have to worry about.
 */

#include <linux/config.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <asm/processor.h>
#include <asm/delay.h>
#include <asm/timer.h>

#ifdef CONFIG_SMP
#include <asm/smp.h>
#endif

extern struct timer_opts* timer;

void __delay(unsigned long loops)
{
   cur_timer->delay(loops);
}

inline void __const_udelay(unsigned long xloops)
{
   int d0;
   __asm__("mull %0"
      :"=d" (xloops), "=&a" (d0)
      :"1" (xloops),"0" (current_cpu_data.loops_per_jiffy));
        __delay(xloops * HZ);
}

void __udelay(unsigned long usecs)
{
   __const_udelay(usecs * 0x000010c6);  /* 2**32 / 1000000 */
}

void __ndelay(unsigned long nsecs)
{
   __const_udelay(nsecs * 0x00005);  /* 2**32 / 1000000000 (rounded up) */
}
