/* $Id: oo_os2fn.c,v 1.2 2005/02/06 22:49:34 smilcke Exp $ */

/*
 * lx_os2fn.c
 * Autor:               Stefan Milcke
 * Erstellt am:         25.04.2002
 * Letzte Aenderung am: 30.01.2005
 *
*/

#include <lxcommon.h>
#include <linux/module.h>
#include <linux/kernel_stat.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/notifier.h>
#include <linux/percpu.h>
#include <linux/cpu.h>
#include <linux/kthread.h>

#include <asm/irq.h>

//---------------------------------- LX_sleep ----------------------------------
int LX_sleep(unsigned long id,unsigned long timeout)
{
 return DevBlock(id,timeout,0);
}

//--------------------------------- LX_wakeup ----------------------------------
int LX_wakeup(unsigned long id,unsigned long data)
{
 data++;
 return DevRun(id);
}

//------------------------------ machine_restart -------------------------------
void  machine_restart(char* cmd)
{
 NOT_IMPLEMENTED();
}

//-------------------------------- machine_halt --------------------------------
void machine_halt(void)
{
 NOT_IMPLEMENTED();
}

//----------------------------- machine_power_off ------------------------------
void machine_power_off(void)
{
 NOT_IMPLEMENTED();
}

//------------------------------ software_suspend ------------------------------
int software_suspend(void)
{
 return -EPERM;
}

