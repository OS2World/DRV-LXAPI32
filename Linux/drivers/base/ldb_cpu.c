/* $Id: ldb_cpu.c,v 1.1 2004/07/19 22:35:55 smilcke Exp $ */

/*
 * ldb_cpu.c
 * Autor:               Stefan Milcke
 * Erstellt am:         22.06.2004
 * Letzte Aenderung am: 23.06.2004
 *
*/

#include <lxcommon.h>
#include <linux/sysdev.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cpu.h>
#include <linux/topology.h>
#include <linux/device.h>

/*
 * drivers/base/cpu.c - basic CPU class support
 */

struct sysdev_class cpu_sysdev_class = {
   set_kset_name("cpu"),
};
EXPORT_SYMBOL(cpu_sysdev_class);

#ifdef CONFIG_HOTPLUG_CPU
static ssize_t show_online(struct sys_device *dev, char *buf)
{
   struct cpu *cpu = container_of(dev, struct cpu, sysdev);

   return sprintf(buf, "%u\n", !!cpu_online(cpu->sysdev.id));
}

static ssize_t store_online(struct sys_device *dev, const char *buf,
             size_t count)
{
   struct cpu *cpu = container_of(dev, struct cpu, sysdev);
   ssize_t ret;

   switch (buf[0]) {
   case '0':
      ret = cpu_down(cpu->sysdev.id);
      break;
   case '1':
      ret = cpu_up(cpu->sysdev.id);
      break;
   default:
      ret = -EINVAL;
   }

   if (ret >= 0)
      ret = count;
   return ret;
}
static SYSDEV_ATTR(online, 0600, show_online, store_online);

static void __init register_cpu_control(struct cpu *cpu)
{
   sysdev_create_file(&cpu->sysdev, &attr_online);
}
#else /* ... !CONFIG_HOTPLUG_CPU */
static inline void register_cpu_control(struct cpu *cpu)
{
}
#endif /* CONFIG_HOTPLUG_CPU */

/*
 * register_cpu - Setup a driverfs device for a CPU.
 * @num - CPU number to use when creating the device.
 *
 * Initialize and register the CPU device.
 */
int __init register_cpu(struct cpu *cpu, int num, struct node *root)
{
   int error;

   cpu->node_id = cpu_to_node(num);
   cpu->sysdev.id = num;
   cpu->sysdev.cls = &cpu_sysdev_class;

   error = sysdev_register(&cpu->sysdev);
   if (!error && root)
      error = sysfs_create_link(&root->sysdev.kobj,
                 &cpu->sysdev.kobj,
                 kobject_name(&cpu->sysdev.kobj));
   if (!error)
      register_cpu_control(cpu);
   return error;
}

int __init cpu_dev_init(void)
{
   return sysdev_class_register(&cpu_sysdev_class);
}
