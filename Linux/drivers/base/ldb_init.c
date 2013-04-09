/* $Id: ldb_init.c,v 1.4 2005/03/01 20:53:15 smilcke Exp $ */

/*
 * ldb_init.c
 * Autor:               Stefan Milcke
 * Erstellt am:         23.06.2004
 * Letzte Aenderung am: 17.02.2005
 *
*/

#include <lxcommon.h>

/*
 *
 * Copyright (c) 2002-3 Patrick Mochel
 * Copyright (c) 2002-3 Open Source Development Labs
 *
 * This file is released under the GPLv2
 *
 */

#include <linux/device.h>
#include <linux/init.h>

extern int devices_init(void);
extern int buses_init(void);
extern int classes_init(void);
extern int firmware_init(void);
extern int platform_bus_init(void);
extern int system_bus_init(void);
extern int cpu_dev_init(void);

/**
 * driver_init - initialize driver model.
 *
 * Call the driver model init functions to initialize their
 * subsystems. Called early from init/main.c.
 */

void __init driver_init(void)
{
   /* These are the core pieces */
   devices_init();
   buses_init();
   classes_init();
   firmware_init();

   /* These are also core pieces, but must come after the
    * core core pieces.
    */
   platform_bus_init();
   system_bus_init();
   cpu_dev_init();
}
