/* $Id: lxsysstate.h,v 1.3 2005/03/22 22:34:32 smilcke Exp $ */

/*
 * lxsysstate.h
 * Autor:               Stefan Milcke
 * Erstellt am:         10.03.2005
 * Letzte Aenderung am: 19.03.2005
 *
*/

#ifndef LXSYSTATE_H_INCLUDED
#define LXSYSTATE_H_INCLUDED

#define LXSYSSTATE_KERNEL_BOOT_STARTED       1
#define LXSYSSTATE_KERNEL_BOOT_FINISHED      2
#define LXSYSSTATE_INIT_STARTED              4
#define LXSYSSTATE_INIT_FINISHED             8

#define LXSYSSTATE_PANIC_ENTERED             512
#define LXSYSSTATE_NOT_INITIALIZED           1024
#define LXSYSSTATE_NOT_OPERABLE              2048
#define LXSYSSTATE_SYSTEM_RUNNING            4096


extern unsigned long lx_sysstate;

extern void LX_set_sysstate(unsigned long setmask,unsigned long clrmask);

#endif //LXSYSTATE_H_INCLUDED
