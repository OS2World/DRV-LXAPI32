/* $Id: oo_symparm.c,v 1.1 2004/09/29 23:27:51 smilcke Exp $ */

/*
 * lx_symparm.c
 * Autor:               Stefan Milcke
 * Erstellt am:         30.06.2004
 * Letzte Aenderung am: 15.08.2004
 *
*/

#include <lxcommon.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/module.h>

#include <lxapi.h>

LIST_HEAD(SYMPARM_segments);
spinlock_t SYMPARM_lock=SPIN_LOCK_UNLOCKED;

//------------------------------ _LX_find_modparm ------------------------------
struct obsolete_modparm* _LX_find_modparm(const char* symname
                                          ,struct obsolete_modparm* pModParm
                                          ,int doExact)
{
 struct list_head* lh;
 struct lx_SYMPARM_segment *pseg;
 struct obsolete_modparm *pFrom=pModParm;
 struct obsolete_modparm *pmp;
 int symnamelen=strlen(symname);
 int i;
 list_for_each(lh,&SYMPARM_segments)
 {
  pseg=list_entry(lh,struct lx_SYMPARM_segment,list);
  pmp=pseg->modparms;
  for(i=0;i<pseg->modparm_count;i++,pmp++)
  {
   if(pFrom)
   {
    if(!strcmp(pmp->name,pFrom->name))
     pFrom=0;
   }
   else if((0==doExact && 0==strncmp(pmp->name,symname,symnamelen))
      || 0==strcmp(pmp->name,symname))
   {
    return pmp;
   }
  }
 }
 return 0;
}

EXPORT_SYMBOL(_LX_find_modparm);

