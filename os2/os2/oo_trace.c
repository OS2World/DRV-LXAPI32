/* $Id: oo_trace.c,v 1.1 2004/09/29 23:27:52 smilcke Exp $ */

/*
 * lx_trace.c
 * Autor:               Stefan Milcke
 * Erstellt am:         06.08.2004
 * Letzte Aenderung am: 14.09.2004
 *
*/

#include <lxcommon.h>
#include <linux/kernel.h>
#include <asm/processor.h>
#include <asm/current.h>
#include <linux/module.h>

extern unsigned long kallsyms_num_syms;

//------------------------------- LX_show_trace --------------------------------
void LX_show_trace(void)
{
 if(kallsyms_num_syms)
 {
  printk("Starting stack trace:\n");
  show_trace(current,0);
 }
}

EXPORT_SYMBOL(LX_show_trace);
