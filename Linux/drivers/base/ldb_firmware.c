/* $Id: ldb_firmware.c,v 1.1 2004/07/19 22:35:56 smilcke Exp $ */

/*
 * ldb_firmware.c
 * Autor:               Stefan Milcke
 * Erstellt am:         23.06.2004
 * Letzte Aenderung am: 23.06.2004
 *
*/

#include <lxcommon.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/init.h>

/*
 * firmware.c - firmware subsystem hoohaw.
 *
 * Copyright (c) 2002-3 Patrick Mochel
 * Copyright (c) 2002-3 Open Source Development Labs
 *
 * This file is released under the GPLv2
 *
 */

static decl_subsys(firmware,NULL,NULL);

int firmware_register(struct subsystem * s)
{
   kset_set_kset_s(s,firmware_subsys);
   return subsystem_register(s);
}

void firmware_unregister(struct subsystem * s)
{
   subsystem_unregister(s);
}

int __init firmware_init(void)
{
   return subsystem_register(&firmware_subsys);
}

EXPORT_SYMBOL(firmware_register);
EXPORT_SYMBOL(firmware_unregister);
