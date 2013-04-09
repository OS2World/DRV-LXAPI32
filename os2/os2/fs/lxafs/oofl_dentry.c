/* $Id: oofl_dentry.c,v 1.5 2005/05/20 22:20:32 smilcke Exp $ */

/*
 * oofl_dentry.c
 * Autor:               Stefan Milcke
 * Erstellt am:         05.02.2005
 * Letzte Aenderung am: 20.05.2005
 *
*/

#define LXAFS_DECLARATIONS
#include <lxcommon.h>

#include "oofl_fs.h"

//-------------------------------- lxafs_d_hash --------------------------------
static int lxafs_d_hash(struct dentry* dentry,struct qstr* qstr)
{
 unsigned long hash;
 int i;
 unsigned l = qstr->len;
 if(l==1) if(qstr->name[0]=='.') goto x;
 if(l==2) if(qstr->name[0]=='.' || qstr->name[1]=='.') goto x;
 lxafs_adjust_length((char*)qstr->name,&l);
x:
 hash=init_name_hash();
 for(i=0;i<l;i++)
  hash=partial_name_hash(lxafs_upcase(qstr->name[i]),hash);
 qstr->hash=end_name_hash(hash);
 return 0;
}

//------------------------------ lxafs_d_compare -------------------------------
static int lxafs_d_compare(struct dentry* dentry,struct qstr* a,struct qstr* b)
{
 unsigned al=a->len;
 unsigned bl=b->len;
 lxafs_adjust_length((char*)a->name,&al);
 if(lxafs_chk_name((char*)b->name,&bl))
  return 1;
 if(lxafs_compare_names(dentry->d_sb,(char*)a->name,al,(char*)b->name,bl,0))
  return 1;
 return 0;
}

static struct dentry_operations lxafs_dentry_operations=
{
 .d_hash       = lxafs_d_hash,
 .d_compare    = lxafs_d_compare,
};

//------------------------ lxafs_set_dentry_operations -------------------------
void lxafs_set_dentry_operations(struct dentry* dentry)
{
 dentry->d_op=&lxafs_dentry_operations;
}
