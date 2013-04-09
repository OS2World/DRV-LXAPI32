/* $Id: ldbp_shutdown.c,v 1.1 2004/07/19 22:35:58 smilcke Exp $ */

/*
 * ldbp_shutdown.c
 * Autor:               Stefan Milcke
 * Erstellt am:         23.06.2004
 * Letzte Aenderung am: 23.06.2004
 *
*/

#include <lxcommon.h>
#include <linux/config.h>
#include <linux/device.h>
#include <asm/semaphore.h>

#include "ldbp_power.h"

/*
 * shutdown.c - power management functions for the device tree.
 *
 * Copyright (c) 2002-3 Patrick Mochel
 *     2002-3 Open Source Development Lab
 *
 * This file is released under the GPLv2
 *
 */

#define to_dev(node) container_of(node,struct device,kobj.entry)

extern struct subsystem devices_subsys;


int device_detach_shutdown(struct device * dev)
{
   if (!dev->detach_state)
      return 0;

   if (dev->detach_state == DEVICE_PM_OFF) {
      if (dev->driver && dev->driver->shutdown)
         dev->driver->shutdown(dev);
      return 0;
   }
   return dpm_runtime_suspend(dev,dev->detach_state);
}


/**
 * We handle system devices differently - we suspend and shut them
 * down first and resume them first. That way, we do anything stupid like
 * shutting down the interrupt controller before any devices..
 *
 * Note that there are not different stages for power management calls -
 * they only get one called once when interrupts are disabled.
 */

extern int sysdev_shutdown(void);

/**
 * device_shutdown - call ->remove() on each device to shutdown.
 */
void device_shutdown(void)
{
   struct device * dev;

   down_write(&devices_subsys.rwsem);
   list_for_each_entry_reverse(dev,&devices_subsys.kset.list,kobj.entry) {
      pr_debug("shutting down %s: ",dev->bus_id);
      if (dev->driver && dev->driver->shutdown) {
         pr_debug("Ok\n");
         dev->driver->shutdown(dev);
      } else
         pr_debug("Ignored.\n");
   }
   up_write(&devices_subsys.rwsem);

   sysdev_shutdown();
}

