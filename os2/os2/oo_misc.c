/* $Id: oo_misc.c,v 1.10 2005/12/26 23:42:06 smilcke Exp $ */

/*
 * oo_misc.c
 * Autor:               Stefan Milcke
 * Erstellt am:         23.01.2005
 * Letzte Aenderung am: 26.12.2005
 *
*/

#include <lxcommon.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/cpumask.h>
#include <linux/config.h>
#include <linux/sched.h>
#include <asm/processor.h>
#include <asm/i387.h>
#include <asm/uaccess.h>
#include <asm/desc.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/radix-tree.h>

extern unsigned long init_pg_tables_end;
extern void LX_init_doublefault(void);

//-------------------------------- LX_init_core --------------------------------
void LX_init_core(void)
{
 if(!init_pg_tables_end==~0UL)
 {
  void *tmp=kmalloc(4096,GFP_KERNEL);
  if(tmp)
  {
   init_pg_tables_end=virt_to_phys(tmp);
  }
 }
 LX_init_doublefault();
}

//-------------------------- LX_set_continue_startup ---------------------------
void LX_set_continue_startup(void)
{
/*
 schedule();
 schedule();
 schedule();
*/
 LX_close_screen();
 *P_INIT_COUNT=0;
}

//------------------------------ LX_set_sysstate -------------------------------
void LX_set_sysstate(unsigned long setmask,unsigned long clrmask)
{
 lx_sysstate|=setmask;
 lx_sysstate&=~clrmask;
}

//-------------------------------- LX_access_ok --------------------------------
// Returns 0, if addr is not ok
//         1, if addr is ok
//         2, if addr is ok, but is in userspace
int LX_access_ok(int type,unsigned long addr,unsigned long size)
{
 if(addr>=0xF0000000)
  return 1;
 return 2;
}
