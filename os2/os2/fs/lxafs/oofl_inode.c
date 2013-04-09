/* $Id: oofl_inode.c,v 1.22 2005/07/25 20:04:25 smilcke Exp $ */

/*
 * oofl_inode.c
 * Autor:               Stefan Milcke
 * Erstellt am:         05.02.2005
 * Letzte Aenderung am: 24.07.2005
 *
*/

#define LXAFS_DECLARATIONS
#include <lxcommon.h>

#include "oofl_fs.h"

//--------------------------- lxafs_mark_inode_dirty ---------------------------
int lxafs_mark_inode_dirty(struct inode* inode,int flags)
{
 if(lxafs_is_lxafs_inode(inode))
 {
  struct lxafs_inode_info* li=LXAFS_I(inode);
  li->dirty_flag|=flags;
 }
 mark_inode_dirty(inode);
 return 0;
}

static kmem_cache_t* lxafs_inode_cachep;

//------------------------------ lxafs_init_once -------------------------------
static void lxafs_init_once(void* foo,kmem_cache_t* cachep,unsigned long flags)
{
 struct lxafs_inode_info* li=(struct lxafs_inode_info*)foo;
 if((flags&(SLAB_CTOR_VERIFY|SLAB_CTOR_CONSTRUCTOR))==SLAB_CTOR_CONSTRUCTOR)
 {
  inode_init_once(&li->inode);
 }
}

//--------------------------- lxafs_init_inodecache ----------------------------
static int __init lxafs_init_inodecache(void)
{
 lxafs_inode_cachep=kmem_cache_create("lxafs_inode_cache"
                                      ,sizeof(struct lxafs_inode_info)
                                      ,0, SLAB_HWCACHE_ALIGN|SLAB_RECLAIM_ACCOUNT
                                      ,lxafs_init_once,0);
 if(lxafs_inode_cachep==0)
  return -ENOMEM;
 return 0;
}

//-------------------------- lxafs_destroy_inodecache --------------------------
static void __exit lxafs_destroy_inodecache(void)
{
 if(kmem_cache_destroy(lxafs_inode_cachep))
  printk(KERN_INFO "lxafs_inode_cache: not all structures were freed\n");
}

//----------------------------- lxafs_alloc_inode ------------------------------
static struct inode* lxafs_alloc_inode(struct super_block* sb)
{
 unsigned long f;
 struct lxafs_inode_info* li;
 struct lxafs_super_info* sbi=LXAFS_SBI(sb);
 li=(struct lxafs_inode_info*)kmem_cache_alloc(lxafs_inode_cachep,SLAB_KERNEL);
 if(!li)
  return 0;
 INIT_LIST_HEAD(&li->linkinfo.list);
 li->linkinfo.name[0]=(char)0;
 li->linkinfo.xname=li->linkinfo.name;
 li->ia_valid=0;
 spin_lock_irqsave(&sbi->inode_list_lock,f);
 list_add(&li->list,&sbi->inode_list);
 spin_unlock_irqrestore(&sbi->inode_list_lock,f);
 return &li->inode;
}

//---------------------------- lxafs_destroy_inode -----------------------------
static void lxafs_destroy_inode(struct inode* inode)
{
 unsigned long f;
 struct lxafs_inode_info* li=LXAFS_I(inode);
 struct lxafs_super_info* sbi=LXAFS_SBI(inode->i_sb);
 struct list_head *lh,*tmp;
 struct lxafs_linkinfo *lfo;
 list_for_each_safe(lh,tmp,&li->linkinfo.list)
 {
  lfo=list_entry(lh,struct lxafs_linkinfo,list);
  list_del(&lfo->list);
  kfree(lfo);
 }
 spin_lock_irqsave(&sbi->inode_list_lock,f);
 list_del(&li->list);
 spin_unlock_irqrestore(&sbi->inode_list_lock,f);
 kmem_cache_free(lxafs_inode_cachep,li);
}

//------------------------------ lxafs_read_inode ------------------------------
static void lxafs_read_inode(struct inode* inode)
{
 struct lxafs_inode_info* li=LXAFS_I(inode);
 if(!lxafs_is_lxafs_inode(inode))
  return;
 if(inode->i_ino==ROOT_I)
 {
  lxafs_get_root_path_i(inode,li->linkinfo.name);
  li->linkinfo.xname=li->linkinfo.name+strlen(LXAFS_ROOTPATH_I(inode));
 }
 else
 {
  DevInt3();
 }
 lxafs_do_read_inode(inode,li->linkinfo.name);
}

/*
static void lxafs_dirty_inode(struct inode* inode)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
}
*/

//------------------------------- lxafs_write_ea -------------------------------
static void lxafs_write_ea(char* eaname,int value,struct inode* inode)
{
 u32 v;
 char val[40];
 unsigned long vallen;
 v=cpu_to_le32(value);
 sprintf(val,"%d",v);
 vallen=strlen(val)+1;
 LX_EAWRITESTRING(LXAFS_I(inode)->linkinfo.name,eaname,val,&vallen);
}

//----------------------------- lxafs_write_inode ------------------------------
void lxafs_write_inode(struct inode* inode,int sync)
{
 struct lxafs_inode_info* li=LXAFS_I(inode);
 // li->is_new
 if(S_ISREG(inode->i_mode))
 {
  if(li->dirty_flag&(LXAFS_DIRTY_LINKLIST | LXAFS_DIRTY_LINKCHILDS))
  {
   lxafs_rewrite_linklist(inode);
   li->dirty_flag&=~(LXAFS_DIRTY_LINKLIST | LXAFS_DIRTY_LINKCHILDS);
  }
  if(li->ia_valid
     &(ATTR_SIZE|ATTR_ATIME|ATTR_MTIME|ATTR_CTIME|ATTR_ATIME_SET|ATTR_MTIME_SET))
  {
   HFILE hFile;
   ULONG rc;
   rc=lxafs_open_inode_for_maintenance(inode,&hFile);
   if(rc>=0)
   {
    if(li->ia_valid&ATTR_SIZE)
    {
     rc=LX_DOSSETFILESIZE(hFile,inode->i_size);
     if(!rc)
      li->ia_valid&=~ATTR_SIZE;
    }
    if(li->ia_valid&(ATTR_ATIME|ATTR_MTIME|ATTR_CTIME|ATTR_ATIME_SET|ATTR_MTIME_SET))
    {
     FILESTATUS4 fs;
     rc=LX_DOSQUERYFILEINFO(hFile,FIL_STANDARD,&fs,sizeof(FILESTATUS3));
     if(!rc)
     {
      if(li->ia_valid&(ATTR_ATIME|ATTR_ATIME_SET))
       lxafs_date_unix2lx(inode->i_atime.tv_sec,&fs.fdateLastAccess,&fs.ftimeLastAccess);
      if(li->ia_valid&(ATTR_MTIME|ATTR_MTIME_SET))
       lxafs_date_unix2lx(inode->i_mtime.tv_sec,&fs.fdateLastWrite,&fs.ftimeLastWrite);
      if(li->ia_valid&(ATTR_CTIME))
       lxafs_date_unix2lx(inode->i_ctime.tv_sec,&fs.fdateCreation,&fs.ftimeCreation);
      rc=LX_DOSSETFILEINFO(hFile,FIL_STANDARD,&fs,sizeof(FILESTATUS3));
     }
     if(!rc)
      li->ia_valid&=~(ATTR_ATIME|ATTR_MTIME|ATTR_CTIME|ATTR_ATIME_SET|ATTR_MTIME_SET);
    }
    LX_DOSCLOSE(hFile);
    if(li->ia_valid&ATTR_MODE)
    {
     lxafs_write_ea(LXEAN_MODE,inode->i_mode,inode);
     li->ia_valid&=~ATTR_MODE;
    }
    if(li->ia_valid&ATTR_UID)
    {
     lxafs_write_ea(LXEAN_UID,inode->i_uid,inode);
     li->ia_valid&=~ATTR_UID;
    }
    if(li->ia_valid&ATTR_GID)
    {
     lxafs_write_ea(LXEAN_GID,inode->i_gid,inode);
     li->ia_valid&=~ATTR_GID;
    }
   }
  }
 }
}

/*
static void lxafs_put_inode(struct inode* inode)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
}
*/

/*
static void lxafs_drop_inode(struct inode* inode)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
}
*/

static void lxafs_delete_inode(struct inode* inode)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 clear_inode(inode);
}

//------------------------------ lxafs_put_super -------------------------------
static void lxafs_put_super(struct super_block* super)
{
 struct lxafs_super_info *sbi=LXAFS_SBI(super);
 kfree(sbi);
}

/*
static void lxafs_write_super(struct super_block* super)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
}
*/

//------------------------------- lxafs_sync_fs --------------------------------
static int lxafs_sync_fs(struct super_block* super,int wait)
{
 // Nothing to do here
 return 0;
}

static void lxafs_write_super_lockfs(struct super_block* super)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
}

static void lxafs_unlockfs(struct super_block* super)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
}

static int lxafs_statfs(struct super_block* super,struct kstatfs* statfs)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return 0;
}

static int lxafs_remount_fs(struct super_block* super,int* i,char* c)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return 0;
}

/*
static void lxafs_clear_inode(struct inode* inode)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
}
*/

/*
static void lxafs_umount_begin(struct super_block* super)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
}
*/

/*
static int lxafs_show_options(struct seq_file* file,struct vfsmount* mnt)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return 0;
}
*/

struct super_operations lxafs_super_operations=
{
 .alloc_inode        = lxafs_alloc_inode,
 .destroy_inode      = lxafs_destroy_inode,
 .read_inode         = lxafs_read_inode,
// .dirty_inode        = lxafs_dirty_inode,
 .write_inode        = lxafs_write_inode,
// .put_inode          = lxafs_put_inode,
// .drop_inode         = lxafs_drop_inode,
 .delete_inode       = lxafs_delete_inode,
 .put_super          = lxafs_put_super,
// .write_super        = lxafs_write_super,
 .sync_fs            = lxafs_sync_fs,
 .write_super_lockfs = lxafs_write_super_lockfs,
 .unlockfs           = lxafs_unlockfs,
 .statfs             = lxafs_statfs,
 .remount_fs         = lxafs_remount_fs,
// .clear_inode        = lxafs_clear_inode,
// .umount_begin       = lxafs_umount_begin,
// .show_options       = lxafs_show_options,
};

//------------------------------ lxafs_fill_super ------------------------------
static int lxafs_fill_super(struct super_block* sb,void* data,int silent)
{
 struct lxafs_super_info* sbi;
 struct inode* root;
 if(strncmp(sb->s_id,"lxda",4))
  return -ENODEV;
 if(!new_valid_dev(sb->s_bdev->bd_dev))
  return -EOVERFLOW;
 sbi=kmalloc(sizeof(struct lxafs_super_info),GFP_KERNEL);
 if(!sbi)
  return -ENOSPC;
 memset(sbi,0,sizeof(struct lxafs_super_info));
 sbi->inode_list_lock=SPIN_LOCK_UNLOCKED;
 INIT_LIST_HEAD(&sbi->inode_list);
 sb->s_fs_info=sbi;
 sb_set_blocksize(sb,PSIZE);
 sb->s_op=&lxafs_super_operations;
 sb->s_magic=LXAFS_SUPER_MAGIC;
 sb->s_maxbytes=4096;
 sb->s_flags&=~(MS_RDONLY);
 sb->s_flags|=MS_SYNCHRONOUS | MS_DIRSYNC;
 lxafs_get_root_path_s(sb,sbi->rootpath);
 root=iget(sb,ROOT_I);
 if(!root || is_bad_inode(root))
  goto out_no_root;
 sb->s_root=d_alloc_root(root);
 if(!sb->s_root)
  goto out_no_root;
 lxafs_set_dentry_operations(sb->s_root);
 return 0;
out_no_root:
 if(root)
  iput(root);
 return 0;
}

LIST_HEAD(lxafs_sblist);

//-------------------------------- lxafs_get_sb --------------------------------
static struct super_block* lxafs_get_sb(struct file_system_type* fs_type
                                        ,int flags
                                        ,const char* dev_name,void* data)
{
 struct super_block* sb;
 sb=get_sb_bdev(fs_type,flags,dev_name,data,lxafs_fill_super);
 if(sb)
 {
  struct lxafs_super_info* sbi=LXAFS_SBI(sb);
  list_add(&sbi->list,&lxafs_sblist);
  {
   struct dentry* sbde;
   struct vfsmount* rootmnt;
   char path[LX_MAXPATH];
   char *ppath;
   sbde=sb->s_root;
   rootmnt=mntget(current->fs->rootmnt);
   ppath=d_path(sbde,rootmnt,path,LX_MAXPATH);
   strncpy(sbi->xrootpath,ppath,LX_MAXPATH);
   mntput(rootmnt);
  }
 }
 return sb;
}

//------------------------------- lxafs_kill_sb --------------------------------
static void lxafs_kill_sb(struct super_block *sb)
{
 struct lxafs_super_info* sbi=LXAFS_SBI(sb);
 if(sbi)
  list_del(&sbi->list);
 kill_block_super(sb);
}

static struct file_system_type lxafs_type=
{
 .owner     = THIS_MODULE,
 .name      = "lxafs",
 .fs_flags  = FS_REQUIRES_DEV,
 .get_sb    = lxafs_get_sb,
 .kill_sb   = lxafs_kill_sb,
};

//--------------------------------- init_lxafs ---------------------------------
static int __init init_lxafs(void)
{
 int rc=lxafs_init_inodecache();
 if(!rc)
  return register_filesystem(&lxafs_type);
 return rc;
}

//--------------------------------- exit_lxafs ---------------------------------
static void __exit exit_lxafs(void)
{
 unregister_filesystem(&lxafs_type);
 lxafs_destroy_inodecache();
}

module_init(init_lxafs)
module_exit(exit_lxafs)
