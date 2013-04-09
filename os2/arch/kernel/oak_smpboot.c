/* $Id: oak_smpboot.c,v 1.3 2005/06/04 23:39:36 smilcke Exp $ */

/*
 * oak_smpboot.c
 * Autor:               Stefan Milcke
 * Erstellt am:         20.07.2004
 * Letzte Aenderung am: 05.06.2005
 *
*/

#include <lxcommon.h>
#include <linux/module.h>
#include <linux/config.h>
#include <linux/init.h>

cpumask_t cpu_online_map=1;
EXPORT_SYMBOL(cpu_online_map);

cpumask_t cpu_callout_map;
EXPORT_SYMBOL(cpu_callout_map);

//------------------------------ smp_alloc_memory ------------------------------
void __init smp_alloc_memory(void)
{
 NOT_IMPLEMENTED();
}
