/* $Id: oofp_lx.c,v 1.3 2005/05/06 23:39:28 smilcke Exp $ */

/*
 * oofp_lx.c
 * Autor:               Stefan Milcke
 * Erstellt am:         03.02.2005
 * Letzte Aenderung am: 07.05.2005
 *
*/

#include <lxcommon.h>
#include <linux/config.h>
#include <linux/buffer_head.h>
#include <linux/hdreg.h>
#include <linux/slab.h>

#include "oofp_check.h"

#include <lxdaemon.h>

//-------------------------------- lx_partition --------------------------------
int lx_partition(struct parsed_partitions *state, struct block_device *bdev)
{
 int i;
 int rc;
 unsigned long diskNum;
 unsigned long driveMap;
 rc=LX_DOSQUERYCURRENTDISK(&diskNum,&driveMap);
 if(!rc && driveMap)
 {
  put_partition(state,1,0,4096);
  printk(KERN_INFO " (root: (points to %s))\n      ",lx_root_path);
/*
  for(i=0;i<26;i++)
  {
   if(test_bit(i,&driveMap))
   {
    put_partition(state,i+2,0,4096);
    printk(KERN_INFO " (%c:)\n      ",(char)(i+'A'));
   }
  }
*/
  printk("\r  \r");
  return 1;
 }
 return 0;
}
