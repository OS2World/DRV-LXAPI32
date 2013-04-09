/* $Id: lakt_timer_none.c,v 1.2 2004/07/20 21:51:51 smilcke Exp $ */

/*
 * lakt_timer_none.c
 * Autor:               Stefan Milcke
 * Erstellt am:         01.07.2004
 * Letzte Aenderung am: 06.07.2004
 *
*/

#include <lxcommon.h>
#include <linux/init.h>
#include <asm/timer.h>

static int __init init_none(char* override)
{
   return 0;
}

static void mark_offset_none(void)
{
   /* nothing needed */
}

static unsigned long get_offset_none(void)
{
   return 0;
}

static unsigned long long monotonic_clock_none(void)
{
   return 0;
}

static void delay_none(unsigned long loops)
{
   int d0;
   __asm__ __volatile__(
      "\tjmp 1f\n"
      ".align 16\n"
      "1:\tjmp 2f\n"
      ".align 16\n"
      "2:\tdecl %0\n\tjns 2b"
      :"=&a" (d0)
      :"0" (loops));
}

/* tsc timer_opts struct */
struct timer_opts timer_none = {
   .name =  "none",
   .init =     init_none,
   .mark_offset = mark_offset_none,
   .get_offset =  get_offset_none,
   .monotonic_clock =   monotonic_clock_none,
   .delay = delay_none,
};
