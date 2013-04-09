/* $Id: ood_lxdskdev.c,v 1.5 2005/02/12 19:47:56 smilcke Exp $ */

/*
 * ood_lxdskdev.c
 * Autor:               Stefan Milcke
 * Erstellt am:         28.12.2004
 * Letzte Aenderung am: 10.02.2005
 *
*/

/* Generic disk access driver for LXAPI32 */

#include <lxcommon.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/dcache.h>
#include <linux/fs.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/genhd.h>
#include <lxdaemon.h>

static int major;
struct lxdisk_data
{
 unsigned long driveMap;
};

static struct gendisk* lxdisk=0;
static struct lxdisk_data lxdisk_data;

static int lxdskdev_ioctl(struct inode* inode,struct file* file,unsigned a,unsigned long b)
{
 DevInt3();
 return 0;
}

static int lxdskdev_media_changed(struct gendisk* disk)
{
 DevInt3();
 return 0;
}

static int lxdskdev_revalidate_disk(struct gendisk* disk)
{
 DevInt3();
 return 0;
}

static struct block_device_operations lxdskdev_ops=
{
 .owner           = THIS_MODULE,
 .ioctl           = lxdskdev_ioctl,
 .media_changed   = lxdskdev_media_changed,
 .revalidate_disk = lxdskdev_revalidate_disk,
};

//------------------------------- lxdskdev_init --------------------------------
static int __init lxdskdev_init(void)
{
 unsigned long rc;
 unsigned long diskNum;
 int i;
 major=register_blkdev(0,"lxd");
 rc=LX_DOSQUERYCURRENTDISK(&diskNum,&lxdisk_data.driveMap);
 if(!rc)
 {
  devfs_mk_dir("lxd");
  if(lxdisk_data.driveMap)
  {
   lxdisk=alloc_disk(28);
   if(lxdisk)
   {
    lxdisk->major=major;
    lxdisk->first_minor=1;
    lxdisk->fops=&lxdskdev_ops;
    lxdisk->private_data=&lxdisk_data;
    sprintf(lxdisk->disk_name,"lxda");
    sprintf(lxdisk->devfs_name,"lxda/0");
    lxdisk->capacity=65536;
    add_disk(lxdisk);
   }
   else
    panic("Unable to allocate root disk\n");
  }
  else
   panic("Root mount point is unreachable\n");
 }
 else
  panic("Failed querying OS/2 drive map\n");
 return rc;
}

subsys_initcall(lxdskdev_init); // Correct? Or after fs_initcall?

