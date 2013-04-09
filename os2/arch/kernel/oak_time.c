/* $Id: oak_time.c,v 1.3 2004/09/29 23:27:35 smilcke Exp $ */

/*
 * oak_time.c
 * Autor:               Stefan Milcke
 * Erstellt am:         26.08.2004
 * Letzte Aenderung am: 25.09.2004
 *
*/

#include <lxcommon.h>

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/smp.h>
#include <linux/module.h>
#include <linux/sysdev.h>
#include <linux/init.h>
#include <asm/irq.h>
#include <asm/timer.h>

unsigned long cpu_khz;  /* Detected as we calibrate the TSC */
spinlock_t i8253_lock = SPIN_LOCK_UNLOCKED;
EXPORT_SYMBOL(i8253_lock);
struct timer_opts* cur_timer=&timer_none;

//------------------------------ do_gettimeofday -------------------------------
void do_gettimeofday(struct timeval *tv)
{
 tv->tv_sec=lx_InfoSegGDTPtr->SIS_BigTime;
 tv->tv_usec=lx_InfoSegGDTPtr->SIS_MsCount;
}

EXPORT_SYMBOL(do_gettimeofday);

//------------------------------ do_settimeofday -------------------------------
int do_settimeofday(struct timespec *tv)
{
 return 0;
}
EXPORT_SYMBOL(do_settimeofday);

/**
 * do_timer_interrupt_hook - hook into timer tick
 * @regs:   standard registers from interrupt
 *
 * Description:
 * This hook is called immediately after the timer interrupt is ack'd.
 * It's primary purpose is to allow architectures that don't possess
 * individual per CPU clocks (like the CPU APICs supply) to broadcast the
 * timer interrupt as a means of triggering reschedules etc.
 **/

static inline void do_timer_interrupt_hook(struct pt_regs *regs)
{
   do_timer(regs);
/*
 * In the SMP case we use the local APIC timer interrupt to do the
 * profiling, except when we simulate SMP mode on a uniprocessor
 * system, in that case we have to call the local interrupt handler.
 */
#ifndef TARGET_OS2
#ifndef CONFIG_X86_LOCAL_APIC
   x86_do_profile(regs);
#else
   if (!using_apic_timer)
      smp_local_timer_interrupt(regs);
#endif
#endif
}

/*
 * timer_interrupt() needs to keep up the real-time clock,
 * as well as call the "do_timer()" routine every clocktick
 */
static inline void do_timer_interrupt(int irq, void *dev_id,
               struct pt_regs *regs)
{
#ifndef TARGET_OS2
#ifdef CONFIG_X86_IO_APIC
   if (timer_ack) {
      /*
       * Subtle, when I/O APICs are used we have to ack timer IRQ
       * manually to reset the IRR bit for do_slow_gettimeoffset().
       * This will also deassert NMI lines for the watchdog if run
       * on an 82489DX-based system.
       */
      spin_lock(&i8259A_lock);
      outb(0x0c, PIC_MASTER_OCW3);
      /* Ack the IRQ; AEOI will end it automatically. */
      inb(PIC_MASTER_POLL);
      spin_unlock(&i8259A_lock);
   }
#endif
#endif

   do_timer_interrupt_hook(regs);
#ifndef TARGET_OS2
   /*
    * If we have an externally synchronized Linux clock, then update
    * CMOS clock accordingly every ~11 minutes. Set_rtc_mmss() has to be
    * called as close as possible to 500 ms before the new second starts.
    */
   if ((time_status & STA_UNSYNC) == 0 &&
       xtime.tv_sec > last_rtc_update + 660 &&
       (xtime.tv_nsec / 1000)
         >= USEC_AFTER - ((unsigned) TICK_SIZE) / 2 &&
       (xtime.tv_nsec / 1000)
         <= USEC_BEFORE + ((unsigned) TICK_SIZE) / 2) {
      /* horrible...FIXME */
      if (efi_enabled) {
         if (efi_set_rtc_mmss(xtime.tv_sec) == 0)
            last_rtc_update = xtime.tv_sec;
         else
            last_rtc_update = xtime.tv_sec - 600;
      } else if (set_rtc_mmss(xtime.tv_sec) == 0)
         last_rtc_update = xtime.tv_sec;
      else
         last_rtc_update = xtime.tv_sec - 600; /* do it again in 60 s */
   }
#endif

#ifndef TARGET_OS2
#ifdef CONFIG_MCA
   if( MCA_bus ) {
      /* The PS/2 uses level-triggered interrupts.  You can't
      turn them off, nor would you want to (any attempt to
      enable edge-triggered interrupts usually gets intercepted by a
      special hardware circuit).  Hence we have to acknowledge
      the timer interrupt.  Through some incredibly stupid
      design idea, the reset for IRQ 0 is done by setting the
      high bit of the PPI port B (0x61).  Note that some PS/2s,
      notably the 55SX, work fine if this is removed.  */

      irq = inb_p( 0x61 ); /* read the current state */
      outb_p( irq|0x80, 0x61 );  /* reset the IRQ */
   }
#endif
#endif
}

//------------------------------ timer_interrupt -------------------------------
irqreturn_t timer_interrupt(int irq,void* dev_id,struct pt_regs* regs)
{
 write_seqlock(&xtime_lock);
 cur_timer->mark_offset();
 do_timer_interrupt(irq,NULL,regs);
 write_sequnlock(&xtime_lock);
 return IRQ_HANDLED;
}

extern void __init time_init_hook(void);

//--------------------------------- time_init ----------------------------------
void __init time_init(void)
{
 struct timeval tv;
 do_gettimeofday(&tv);
 xtime.tv_sec=tv.tv_sec;
 wall_to_monotonic.tv_sec=-xtime.tv_sec;
 xtime.tv_nsec=(INITIAL_JIFFIES % HZ) * (NSEC_PER_SEC / HZ);
 wall_to_monotonic.tv_nsec=-xtime.tv_nsec;
 cur_timer=select_timer();
 printk(KERN_INFO "Using %s for high-res timesource\n",cur_timer->name);
 time_init_hook();
}
