/* $Id: ol_misc.c,v 1.8 2005/06/09 22:25:25 smilcke Exp $ */

/*
 * misc.c
 * Autor:               Stefan Milcke
 * Erstellt am:         08.11.2001
 * Letzte Aenderung am: 09.06.2005
 *
*/

#include <lxcommon.h>
#include <linux/init.h>
#include <asm/hardirq.h>

//--------------------------------- dump_stack ---------------------------------
void dump_stack(void)
{
}

//----------------------------------- panic ------------------------------------
NORET_TYPE void panic(const char* fmt, ...)
{
 va_list args;
 printk(KERN_EMERG "**** PANIC! ****\n");
 LX_set_sysstate(LXSYSSTATE_NOT_OPERABLE,0);
 va_start(args,fmt);
 printk(fmt,args);
 va_end(args);
 if(!in_interrupt())
 {
  if(P_INIT_COUNT && 0!=*P_INIT_COUNT)
  {
   LX_wait_for_stable_memstat(LX_WFSM_PRINTALL);
   printk("\nTrying to continue system startup\n");
   LX_set_continue_startup();
  }
  if(!(lx_sysstate&LXSYSSTATE_PANIC_ENTERED))
  {
   LX_set_sysstate(LXSYSSTATE_PANIC_ENTERED,0);
   for(;;)
    schedule();
  }
  else
  {
   LX_set_sysstate(LXSYSSTATE_PANIC_ENTERED,0);
   atomic_set(&lx_current_available,0);
   for(;;)
    DevBlock((unsigned long)current,10000,0);
  }
 }
 else
 {
  atomic_set(&lx_current_available,0);
  for(;;);
 }
}
