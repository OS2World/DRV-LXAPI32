/* $Id: oofl_dir.c,v 1.33 2005/07/25 20:04:24 smilcke Exp $ */

/*
 * oofl_dir.c
 * Autor:               Stefan Milcke
 * Erstellt am:         04.02.2005
 * Letzte Aenderung am: 24.07.2005
 *
*/

#define LXAFS_DECLARATIONS
#include <lxcommon.h>

#include "oofl_fs.h"

//-------------------------------- get_new_ino ---------------------------------
static int get_new_ino(void)
{
 static int g_ino=3;
 int ino=g_ino;
 g_ino++;
 if(g_ino==0)
  g_ino=3;
 return ino;
}

//--------------------------------- iod_create ---------------------------------
static int
 iod_create(struct inode* inode,struct dentry* dentry,int i,struct nameidata* nameidata)
{
 const char* name=dentry->d_name.name;
 unsigned len=dentry->d_name.len;
 struct inode* result=NULL;
 struct lxafs_inode_info* li=LXAFS_I(inode);
 struct lxafs_inode_info* result_li;
 struct lxafs_fileinfo* fileinfo;
 int err;
 char* fullname=kmalloc(LX_MAXPATH,GFP_KERNEL);
 if(!fullname)
  return -ENOMEM;
 lxafs_build_fullname_from_parent_i(fullname,inode,(char*)name,LXAFS_BUILDFNAME_S);
 if((err=lxafs_chk_name((char*)name,&len)))
 {
  kfree(fullname);
  return err==-ENOENT ? -EINVAL : err;
 }
 lock_kernel();
 result=new_inode(inode->i_sb);
 if(!result)
  goto bail1;
 result_li=LXAFS_I(result);
 strcpy(result_li->linkinfo.name,fullname);
 result_li->linkinfo.xname=result_li->linkinfo.name+strlen(LXAFS_ROOTPATH_I(inode));
 result->i_ino=get_new_ino();
 err=lxafs_create_file(fullname);
 if(err)
  goto bail2;
 fileinfo=lxafs_get_fullfileinfo(fullname,LXAFS_ROOTPATH_I(inode));
 if(fileinfo)
 {
  lxafs_init_inode(result,fileinfo);
  lxafs_free_fileinfo(fileinfo);
 }
 result->i_mode&=~0111;
 insert_inode_hash(result);
 lxafs_set_dentry_operations(dentry);
 d_instantiate(dentry,result);
 unlock_kernel();
 kfree(fullname);
 return 0;
bail2:
 iput(result);
bail1:
bail:
 unlock_kernel();
 kfree(fullname);
 return err;
}

//---------------------------- lxafs_lookup_create -----------------------------
static struct dentry* lxafs_lookup_create(struct nameidata *nd,int is_dir)
{
 struct dentry *dentry;
 dentry = ERR_PTR(-EEXIST);
 if (nd->last_type != LAST_NORM)
  goto fail;
 nd->flags &= ~LOOKUP_PARENT;
 dentry = lookup_hash(&nd->last, nd->dentry);
 if (IS_ERR(dentry))
  goto fail;
 if (!is_dir && nd->last.name[nd->last.len] && !dentry->d_inode)
  goto enoent;
 return dentry;
enoent:
 dput(dentry);
 dentry = ERR_PTR(-ENOENT);
fail:
 return dentry;
}

//--------------------------- lxafs_add_link_childs ----------------------------
static void lxafs_add_link_childs(struct lxafs_fileinfo* fileinfo
                                  ,struct inode* inode
                                  ,struct dentry* dentry)
{
 struct list_head *lh,*tmp;
 struct lxafs_linkinfo *lfo;
 list_for_each_safe(lh,tmp,&fileinfo->linkinfo.list)
 {
  int error;
  struct dentry* new_dentry;
  struct nameidata nd;
  lfo=list_entry(lh,struct lxafs_linkinfo,list);
  list_del(&lfo->list);
  // Check, if someone has deleted the file
  if(lxafs_file_exists(lfo->name))
   if(!lxafs_add_linkinfo(inode,lfo,1))
    lfo=0;
  if(lfo)
   kfree(lfo);
 }
 lxafs_mark_inode_dirty(inode,LXAFS_DIRTY_LINKLIST);
}

//-------------------------- lxafs_linkmaster_lookup ---------------------------
static struct dentry*
 lxafs_linkmaster_lookup(struct inode* dir
                         ,struct dentry* dentry
                         ,struct nameidata* nameidata
                         ,struct lxafs_fileinfo* fileinfo
                         ,char* fullname)
{
 struct nameidata nd;
 struct dentry* result=ERR_PTR(-ENOENT);
 int error;
 if(lxafs_file_exists(fileinfo->linkinfo.name))
 {
  char xname[LX_MAXPATH];
  strncpy(xname,LXAFS_XROOTPATH_I(dir),LX_MAXPATH);
  strncat(xname,fileinfo->linkinfo.xname,LX_MAXPATH-strlen(xname));
  lock_kernel();
  up(&dir->i_sem);
  error=path_lookup(xname,0,&nd);
  if(!error)
  {
   if(nd.dentry && nd.dentry->d_inode)
    result=nd.dentry;
   path_release(&nd);
  }
  down(&dir->i_sem);
  unlock_kernel();
 }
 if(IS_ERR(result))
  printk(KERN_WARNING "%s's link master not found!\n",fullname);
 return result;
}

//--------------------------------- iod_lookup ---------------------------------
static struct dentry*
 iod_lookup(struct inode* dir,struct dentry* dentry,struct nameidata* nameidata)
{
 const char* name=dentry->d_name.name;
 int len=dentry->d_name.len;
 ino_t ino;
 int err;
 struct inode* result=NULL;
 struct lxafs_inode_info* result_li=NULL;
 struct lxafs_inode_info* li=LXAFS_I(dir);
 struct lxafs_fileinfo *fileinfo=NULL;
 char *fullname=kmalloc(LX_MAXPATH,GFP_KERNEL);
 if(!fullname)
  return ERR_PTR(-ENOMEM);
 lxafs_build_fullname_from_parent_i(fullname,dir,(char*)name,LXAFS_BUILDFNAME_S);
 lock_kernel();
 if((err=lxafs_chk_name((char*)name,&len)))
 {
  if(err==-ENAMETOOLONG)
  {
   unlock_kernel();
   kfree(fullname);
   return ERR_PTR(-ENAMETOOLONG);
  }
  goto end_add;
 }
 fileinfo=lxafs_get_fullfileinfo(fullname,LXAFS_ROOTPATH_I(dir));
 if(!nameidata)
 {
  if(fileinfo)
   goto do_it;
  else
   goto end_noadd;
 }
 if((nameidata->flags & LOOKUP_CREATE))
 {
  if(fileinfo && (nameidata->intent.open.flags&O_TRUNC))
   goto do_it;
  if(fileinfo)
  {
   DevInt3();
   unlock_kernel();
   kfree(fullname);
   return ERR_PTR(-EEXIST);
  }
  goto end_noadd;
 }
 else if(!fileinfo
         && (nameidata->flags&(LOOKUP_OPEN))
         && !(nameidata->intent.open.flags&O_TRUNC))
  goto bail1;
 else if(!fileinfo
         && !(nameidata->flags&(LOOKUP_CREATE)))
  goto bail1;
do_it:
 if(fileinfo && fileinfo->linkinfo.name[0]!=(char)0)
 {
  struct dentry* mdentry;
  unlock_kernel();
  mdentry=lxafs_linkmaster_lookup(dir,dentry,nameidata,fileinfo,li->linkinfo.name);
  lock_kernel();
  if(IS_ERR(mdentry))
   goto end_noadd;
  simple_link(mdentry,dir,dentry);
  lxafs_set_dentry_operations(dentry);
  d_rehash(dentry);
  goto end_noadd;
 }
 ino=get_new_ino();
 result=iget_locked(dir->i_sb,ino);
 if(!result)
  goto bail1;
 result_li=LXAFS_I(result);
 if(fileinfo && fileinfo->linkinfo.name[0]!=(char)0)
  strncpy(result_li->linkinfo.name,fileinfo->linkinfo.name,LX_MAXPATH);
 else
  strncpy(result_li->linkinfo.name,fullname,LX_MAXPATH);
 result_li->linkinfo.xname=result_li->linkinfo.name+strlen(LXAFS_ROOTPATH_I(dir));
 if(result->i_state & I_NEW)
 {
  lxafs_init_inode(result,fileinfo);
  if(result->i_mode==0)
  {
   result->i_mode |= S_IFREG;
   result->i_mode &= ~0111;
   result->i_nlink=1;
  }
  unlock_new_inode(result);
 }
end_add:
 lxafs_set_dentry_operations(dentry);
 unlock_kernel();
 d_add(dentry,result);
 if(fileinfo && !list_empty(&fileinfo->linkinfo.list))
  lxafs_add_link_childs(fileinfo,result,dentry);
 goto end_free;
end_noadd:
 unlock_kernel();
end_free:
 lxafs_free_fileinfo(fileinfo);
 kfree(fullname);
 return 0;
bail1:
 unlock_kernel();
bail_last:
 lxafs_free_fileinfo(fileinfo);
 kfree(fullname);
 return ERR_PTR(-ENOENT);
}

#define LX_MAX_LINKS (16)
//---------------------------------- iod_link ----------------------------------
static int
 iod_link(struct dentry* old_dentry,struct inode* dir,struct dentry* dentry)
{
 int ret=-EPERM;
 struct lxafs_linkinfo* linkmaster;
 struct lxafs_linkinfo* link;
 // limit number of links
 if(old_dentry->d_inode->i_nlink>LX_MAX_LINKS-1)
  return -EMLINK;
 linkmaster=lxafs_alloc_linkinfo(strlen(LXAFS_ROOTPATH_I(dir)));
 link=lxafs_alloc_linkinfo(strlen(LXAFS_ROOTPATH_I(dir)));
 if(!linkmaster || !link)
  goto bail;
 lxafs_build_fullname_from_parent_i(link->name,dir,(char*)dentry->d_iname
                                    ,LXAFS_BUILDFNAME_S);
 if(!lxafs_create_file(link->name))
 {
  simple_link(old_dentry,dir,dentry);
  lxafs_set_dentry_operations(dentry);
  d_rehash(dentry);
  lxafs_build_fullname_i(linkmaster->name,LX_MAXPATH,old_dentry->d_inode
                         ,LXAFS_BUILDFNAME_S);
  if(!lxafs_write_linkchild_info(link->name,linkmaster->name))
   lxafs_attach_linked_file(linkmaster,link);
  ret=0;
 }
bail:
 lxafs_free_linkinfo(linkmaster);
 lxafs_free_linkinfo(link);
 return ret;
}

//--------------------------------- iod_unlink ---------------------------------
static int
 iod_unlink(struct inode* dir,struct dentry* dentry)
{
 int rc=-ENOENT;
 struct lxafs_linkinfo* linkmaster;
 struct lxafs_linkinfo* link;
 struct inode* inode=dentry->d_inode;
 linkmaster=lxafs_alloc_linkinfo(strlen(LXAFS_ROOTPATH_I(dir)));
 link=lxafs_alloc_linkinfo(strlen(LXAFS_ROOTPATH_I(dir)));
 if(!linkmaster || !link)
  goto bail;
 lxafs_build_fullname_from_parent_i(link->name,dir,(char*)dentry->d_iname
                                    ,LXAFS_BUILDFNAME_S);
 lxafs_build_fullname_i(linkmaster->name,LX_MAXPATH,dentry->d_inode
                        ,LXAFS_BUILDFNAME_S);
 if(!strncmp(link->name,linkmaster->name,LX_MAXPATH))
 {
  struct lxafs_inode_info* li=LXAFS_I(inode);
  struct lxafs_linkinfo* lfo;
  struct list_head* lh;
  if(!list_empty(&li->linkinfo.list))
  {
   lfo=list_entry(li->linkinfo.list.prev,struct lxafs_linkinfo,list);
   LX_DOSCOPY(link->name,lfo->name,DCPY_FAILEAS|DCPY_EXISTING);
   if(S_ISDIR(dentry->d_inode->i_mode))
    rc=LX_DOSDELETEDIR(link->name);
   else
    rc=LX_DOSDELETE(link->name);
   if(!rc)
   {
    LX_EADELETE(lfo->name,LXEAN_LINKEDTO);
    lxafs_free_linkinfo(linkmaster);
    linkmaster=lfo;
    strncpy(li->linkinfo.name,linkmaster->name,LX_MAXPATH);
    list_del(&linkmaster->list);
    list_for_each(lh,&li->linkinfo.list)
    {
     lfo=list_entry(lh,struct lxafs_linkinfo,list);
     lxafs_write_linkchild_info(lfo->name,linkmaster->name);
    }
    lxafs_mark_inode_dirty(dentry->d_inode,LXAFS_DIRTY_LINKLIST);
   }
  }
  else
  {
   if(S_ISDIR(dentry->d_inode->i_mode))
    rc=LX_DOSDELETEDIR(link->name);
   else
    rc=LX_DOSDELETE(link->name);
  }
 }
 else
 {
  lxafs_detach_linked_file(linkmaster,link);
  if(S_ISDIR(dentry->d_inode->i_mode))
   rc=LX_DOSDELETEDIR(link->name);
  else
   rc=LX_DOSDELETE(link->name);
  simple_unlink(dir,dentry);
 }
 if(rc)
  rc=-EACCES;
bail:
 lxafs_free_linkinfo(linkmaster);
 lxafs_free_linkinfo(link);
 return rc;
}

static int
 iod_symlink(struct inode* inode,struct dentry* dentry,const char* c)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

//--------------------------------- iod_mkdir ----------------------------------
static int
 iod_mkdir(struct inode* dir,struct dentry* dentry,int mode)
{
 const char* name=dentry->d_name.name;
 unsigned len=dentry->d_name.len;
 struct inode* result;
 struct lxafs_inode_info* result_li;
 int err=-ENOENT;
 char* fullname=kmalloc(LX_MAXPATH,GFP_KERNEL);
 if(!fullname)
  return -ENOMEM;
 if((err=lxafs_chk_name((char*)name,&len)))
  return err==-ENOENT ? -EINVAL : err;
 lxafs_build_fullname_from_parent_i(fullname,dir,(char*)name,LXAFS_BUILDFNAME_S);
 lock_kernel();
 if(LX_DOSCREATEDIR(fullname,NULL))
  goto bail1;
 result=new_inode(dir->i_sb);
 if(!result)
  goto bail1;
 result_li=LXAFS_I(result);
 result->i_op=&lxafs_dir_iops;
 result->i_fop=&lxafs_dir_fops;
 result->i_size=2048;
 result->i_nlink=2;
 result->i_mode|=S_IFDIR;
 dir->i_nlink++;
 strncpy(result_li->linkinfo.name,fullname,LX_MAXPATH);
 result_li->linkinfo.xname=result_li->linkinfo.name+strlen(LXAFS_ROOTPATH_I(dir));
 insert_inode_hash(result);
 d_instantiate(dentry,result);
 d_rehash(dentry);
 err=0;
 goto out;
bail:
 iput(result);
bail1:
 err=-ENOENT;
out:
 unlock_kernel();
out_nounlock:
 if(fullname)
  kfree(fullname);
 return err;
}

//--------------------------------- iod_rmdir ----------------------------------
static int
 iod_rmdir(struct inode* dir,struct dentry* dentry)
{
 const char* name=dentry->d_name.name;
 int err;
 char *fullname=kmalloc(LX_MAXPATH,GFP_KERNEL);
 if(!fullname)
  return -ENOMEM;
 lxafs_build_fullname_from_parent_i(fullname,dir,(char*)name,LXAFS_BUILDFNAME_S);
 err=LX_DOSDELETEDIR(fullname);
 kfree(fullname);
 if(err)
  return -EACCES;
 return err;
}

static int
 iod_mknod(struct inode* inode,struct dentry* dentry,int i,dev_t dev)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

//-------------------------------- lxafs_rename --------------------------------
static int lxafs_rename(struct inode* old_dir,struct dentry* old_dentry
                        ,struct inode* new_dir,struct dentry* new_dentry)
{
 int err;
 struct inode* inode=old_dentry->d_inode;
 char* oldname;
 char* newname;
 char* tmp;
 err=LX_alloc_objects(3,LX_MAXPATH,GFP_KERNEL,&oldname,&newname,&tmp);
 if(err)
  return err;
 lxafs_build_fullname_from_parent_i(oldname,old_dir,(char*)old_dentry->d_name.name
                                    ,LXAFS_BUILDFNAME_S);
 lxafs_build_fullname_from_parent_i(newname,new_dir,(char*)new_dentry->d_name.name
                                    ,LXAFS_BUILDFNAME_S);
 // Performance note:
 // Rename and do a really FS deep scan of all files and subdirs and change
 // the linkinfos if neccessary. We delegate this completely to the daemon
 // because it can result in a lot of io operations with a schedule() for
 // every io operation if we do it here.
 err=LX_LXAFS_RENAME(oldname,newname);
 if(!err)
 {
  strncpy(LXAFS_I(inode)->linkinfo.name,newname,LX_MAXPATH);
  lxafs_mark_inode_dirty(inode,LXAFS_DIRTY_LINKLIST|LXAFS_DIRTY_LINKCHILDS);
  lxafs_write_inode(inode,1);
  if(S_ISDIR(inode->i_mode))
  {
   // iterate through all known inodes and change link infos
   {
    struct list_head *lh;
    struct lxafs_inode_info* li;
    struct lxafs_super_info* sbi=LXAFS_SBI(inode->i_sb);
    list_for_each(lh,&(sbi->inode_list))
    {
     li=list_entry(lh,struct lxafs_inode_info,list);
     if(!strncmp(li->linkinfo.name,oldname,strlen(oldname)))
     {
      strncpy(tmp,&li->linkinfo.name[strlen(oldname)],LX_MAXPATH);
      strncpy(li->linkinfo.name,newname,LX_MAXPATH);
      strncat(li->linkinfo.name,tmp,LX_MAXPATH-strlen(li->linkinfo.name));
      lxafs_mark_inode_dirty((struct inode*)li
                             ,LXAFS_DIRTY_LINKLIST|LXAFS_DIRTY_LINKCHILDS);
      lxafs_write_inode((struct inode*)li,1);
     }
    }
   }
  }
 }
 kfree(oldname);
 return err;
}

//--------------------------------- iod_rename ---------------------------------
static int
 iod_rename(struct inode* old_dir,struct dentry* old_dentry
            ,struct inode* new_dir,struct dentry* new_dentry)
{
 int err=0;
 struct inode* old_inode;
 struct inode* new_inode;
 err=lxafs_rename(old_dir,old_dentry,new_dir,new_dentry);
 if(!err)
 {
  old_inode=old_dentry->d_inode;
  new_inode=old_dentry->d_inode;
  err=simple_rename(old_dir,old_dentry,new_dir,new_dentry);
//  mark_inode_dirty(old_inode);
  mark_inode_dirty(old_dir);
  if(new_inode)
   mark_inode_dirty(new_inode);
  mark_inode_dirty(new_dir);
  d_rehash(old_dentry);
  d_rehash(new_dentry);
 }
 else
  err=-EBUSY;
 return err;
}

/*
static int
 iod_readlink(struct dentry* dentry, char __user* c,int i)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static int
 iod_follow_link(struct dentry* dentry,struct nameidata* nameidata)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static void
 iod_truncate(struct inode* inode)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
}
*/

/*
static int
 iod_permission(struct inode* inode,int i,struct nameidata* nameidata)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return 0;
}
*/

static int
 iod_setattr(struct dentry* dentry,struct iattr* iattr)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

static int
 iod_getattr(struct vfsmount* mnt,struct dentry* dentry,struct kstat* kstat)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

static int
 iod_setxattr(struct dentry* dentry,const char* c,const void* data,size_t len,int i)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

static ssize_t
 iod_getxattr(struct dentry* dentry,const char* c,void* data,size_t len)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

static ssize_t
 iod_listxattr(struct dentry* dentry,char* c,size_t len)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

static int
 iod_removexattr(struct dentry* dentry,const char* c)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

static loff_t fod_llseek(struct file *filp, loff_t offset, int origin)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

/*
static ssize_t
 fod_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static ssize_t
 fod_aio_read(struct kiocb *iocb, char __user *buf, size_t count, loff_t pos)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static ssize_t
 fod_write(struct file *filp, const char __user *buf,size_t count, loff_t *ppos)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static ssize_t
 fod_aio_write(struct kiocb *iocb, const char __user *buf,size_t count, loff_t pos)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

//-------------------------------- fod_readdir ---------------------------------
static int
 fod_readdir(struct file * filp, void * dirent, filldir_t filldir)
{
 int rc=0;
 struct lxafs_filep_priv_d* pf=filp->private_data;
 struct inode* inode=filp->f_dentry->d_inode;
 struct lxafs_dirent* dent;
 if(filp->f_pos>LXAFS_MAX_DENTS)
  return 0;
 if(pf)
 {
  if((filp->f_pos>=pf->num_ents))
   return 0;
  dent=&(pf->dirents[filp->f_pos]);
  while(dent)
  {
   int slen=strlen(dent->name);
   if(dent->ino==LXAFS_INO_UNDEFINED)
   {
    if(dent->name[0]=='.')
    {
     if(dent->name[1]==(char)0)
      dent->ino=inode->i_ino;
     else if(dent->name[1]=='.' && dent->name[2]==(char)0)
      dent->ino=filp->f_dentry->d_parent->d_inode->i_ino;
    }
    if(dent->ino==LXAFS_INO_UNDEFINED)
    {
     struct nameidata nd;
     int sem_state;
/*
     char xname[LX_MAXPATH];
     strncpy(xname,LXAFS_XROOTPATH_I(dir->i_sb),LX_MAXPATH);
     strncat(xname,fileinfo->linkinfo.xname,LX_MAXPATH-strlen(xname));
*/
     lock_kernel();
     sem_state=atomic_read(&inode->i_sem.count);
     if(!sem_state)
      up(&inode->i_sem);
     rc=path_lookup(dent->xname,LOOKUP_ACCESS|LOOKUP_NOALT,&nd);
     if(rc)
     {
      if(!sem_state && sem_state!=atomic_read(&inode->i_sem.count))
       down(&inode->i_sem);
      unlock_kernel();
      goto out;
     }
     if(nd.dentry && nd.dentry->d_inode)
      dent->ino=nd.dentry->d_inode->i_ino;
     else
      dent->ino=LXAFS_INO_UNKNOWN;
     path_release(&nd);
     if(!sem_state && sem_state!=atomic_read(&inode->i_sem.count))
      down(&inode->i_sem);
     unlock_kernel();
    }
   }
   if(filldir(dirent,dent->name,slen,filp->f_pos,dent->ino,dent->type)<0)
    goto out;
   filp->f_pos++;
   if((filp->f_pos>LXAFS_MAX_DENTS) || (filp->f_pos>=pf->num_ents))
    dent=0;
   else
    dent=&(pf->dirents[filp->f_pos]);
  }
 }
out:
 return rc;
}

/*
static unsigned int
 fod_poll(struct file *filp, struct poll_table_struct *wait)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static int
 fod_ioctl(struct inode * inode, struct file * filp,unsigned int cmd, unsigned long arg)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static int
 fod_mmap(struct file * filp, struct vm_area_struct * vma)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

//------------------------------ lxafs_reopen_dir ------------------------------
static int lxafs_reopen_dir(struct file* filp)
{
 int rc=-ENOMEM;
 int i,j;
 struct lxafs_fileinfo* fi=0;
 struct lxafs_filep_priv_d* pf=filp->private_data;
 struct lxafs_dirent*   dirinfo=0;
 struct lxafs_dirent*   dirents=0;
 struct inode* inode=filp->f_dentry->d_inode;
 struct lxafs_inode_info* li=LXAFS_I(filp->f_dentry->d_inode);
 HDIR hDir=HDIR_CREATE;
 FILEFINDBUF4 *pfb=0;
 ULONG cFileNames=1;
 char* name=0;
 char* fspec;
 char* tmp;
 fi=lxafs_get_fullfileinfo(li->linkinfo.name,LXAFS_ROOTPATH_I(inode));
 if(!fi)
  goto out_err;
 pf->num_ents=fi->num_files+fi->num_subdirs;
 dirinfo=kmalloc(sizeof(struct lxafs_dirent),GFP_KERNEL);
 dirents=kmalloc(sizeof(struct lxafs_dirent)*pf->num_ents,GFP_KERNEL);
 pfb=kmalloc(sizeof(FILEFINDBUF4),GFP_KERNEL);
 if(!(dirinfo) || !(dirents) || !(pfb))
  goto out_err;
 if(LX_alloc_objects(2,LX_MAXPATH,GFP_KERNEL,&name,&fspec))
  goto out_err;
 i=0;
 rc=-ENOENT;
 strncpy(name,li->linkinfo.name,LX_MAXPATH);
 strcat(name,"/*");
 strncpy(fspec,li->linkinfo.name,LX_MAXPATH);
 strcat(fspec,"/");
 tmp=fspec+strlen(fspec);
 if(LX_DOSFINDFIRST(name,&hDir
                    ,FILE_DIRECTORY|FILE_HIDDEN|FILE_SYSTEM|FILE_NORMAL
                    ,pfb,sizeof(FILEFINDBUF4),&cFileNames,FIL_QUERYEASIZE))
   goto out_err;
 rc=0;
 while(cFileNames && i<pf->num_ents)
 {
  strcpy(tmp,pfb->achName);
  strncpy(dirents[i].fullname,fspec,LX_MAXPATH);
  dirents[i].xname=dirents[i].fullname+strlen(LXAFS_ROOTPATH_I(inode));
  dirents[i].name=dirents[i].fullname+strlen(dirents[i].fullname)-strlen(pfb->achName);
  dirents[i].ino=LXAFS_INO_UNDEFINED;
  dirents[i].type=(pfb->attrFile&FILE_DIRECTORY) ? DT_DIR : DT_REG;
  i++;
  if(LX_DOSFINDNEXT(hDir,pfb,sizeof(FILEFINDBUF4),&cFileNames))
   break;
 }
 LX_DOSFINDCLOSE(hDir);
 // Sort dirents
 for(i=0;i<pf->num_ents-1;i++)
 {
  for(j=i+1;j<pf->num_ents;j++)
  {
   if(strncmp(dirents[i].xname,dirents[i].xname,LX_MAXPATH)>0)
   {
    struct lxafs_dirent dt;
    memcpy(&dt,&dirents[i],sizeof(struct lxafs_dirent));
    memcpy(&dirents[i],&dirents[j],sizeof(struct lxafs_dirent));
    memcpy(&dirents[j],&dt,sizeof(struct lxafs_dirent));
    i=0;
    j=0;
   }
  }
 }
 if(pf->dirinfo)
  kfree(pf->dirinfo);
 if(pf->dirents)
  kfree(pf->dirents);
 pf->dirinfo=dirinfo;
 pf->dirents=dirents;
 filp->f_pos=0;
 dirinfo=0;
 dirents=0;
out:
out_err:
 if(fi)
  lxafs_free_fileinfo(fi);
 if(dirinfo)
  kfree(dirinfo);
 if(dirents)
  kfree(dirents);
 if(pfb)
  kfree(pfb);
 if(name)
  kfree(name);
 return rc;
}

//---------------------------------- fod_open ----------------------------------
static int
 fod_open(struct inode* inode, struct file * filp)
{
 int rc=-ENOMEM;
 struct lxafs_filep_priv_d* pf;
 pf=kmalloc(sizeof(struct lxafs_filep_priv_d),GFP_KERNEL);
 if((!pf))
  return -ENOMEM;
 pf->old_dirinfo=0;
 pf->old_dirents=0;
 pf->dirinfo=0;
 pf->dirents=0;
 filp->private_data=pf;
 rc=lxafs_reopen_dir(filp);
 return rc;
}

/*
static int
 fod_flush(struct file *filp)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

//-------------------------------- fod_release ---------------------------------
static int
 fod_release(struct inode* inode, struct file* filp)
{
 struct lxafs_filep_priv_d* pf;
 pf=filp->private_data;
 if(pf)
 {
  if(pf->dirents)
   kfree(pf->dirents);
  if(pf->dirinfo)
   kfree(pf->dirinfo);
  kfree(pf);
  filp->private_data=0;
 }
 return 0;
}

//--------------------------------- fod_fsync ----------------------------------
static int
 fod_fsync(struct file* filp, struct dentry *dentry, int datasync)
{
 int rc;
 if(filp->private_data)
  return lxafs_reopen_dir(filp);
 else
  return -ENOMEM;
}

/*
static int
 fod_fasync(int fd, struct file *filp, int on)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static int
 fod_lock(struct file *filp, int cmd, struct file_lock *fl)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static ssize_t
 fod_readv(struct file *file, const struct iovec *vector,unsigned long count, loff_t *ppos)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static ssize_t
 fod_writev(struct file *file, const struct iovec *vector,unsigned long count, loff_t *ppos)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static ssize_t
 fod_sendfile(struct file *in_file, loff_t *ppos,size_t count, read_actor_t actor, void __user *target)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static ssize_t
 fod_sendpage(struct file *file, struct page *page,int offset, size_t size, loff_t *ppos, int more)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static unsigned long
 fod_get_unmapped_area(struct file *file, unsigned long addr
                       ,unsigned long len, unsigned long pgoff, unsigned long flags)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static long
 fod_fcntl(int file_desc, unsigned int command, unsigned long arg,struct file * file)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

struct file_operations lxafs_dir_fops=
{
 .owner=THIS_MODULE,
 .llseek=fod_llseek,
// .read=fod_read,
 .read=generic_read_dir,
// .aio_read=fod_aio_read,
// .write=fod_write,
// .aio_write=fod_aio_write,
 .readdir=fod_readdir,
// .poll=fod_poll,
// .ioctl=fod_ioctl,
// .mmap=fod_mmap,
 .open=fod_open,
// .flush=fod_flush,
 .release=fod_release,
 .fsync=fod_fsync,
// .aio_fsync=fod_aio_fsync,
// .fasync=fod_fasync,
// .lock=fod_lock,
// .readv=fod_readv,
// .writev=fod_writev,
// .sendfile=fod_sendfile,
// .sendpage=fod_sendpage,
// .get_unmapped_area=fod_get_unmapped_area,
// .fcntl=fod_fcntl,
};

struct inode_operations lxafs_dir_iops=
{
 .create=iod_create,
 .lookup=iod_lookup,
 .link=iod_link,
 .unlink=iod_unlink,
 .symlink=iod_symlink,
 .mkdir=iod_mkdir,
 .rmdir=iod_rmdir,
 .mknod=iod_mknod,
 .rename=iod_rename,
// .readlink=iod_readlink,
// .follow_link=iod_follow_link,
// .truncate=iod_truncate,
// .permission=iod_permission,
 .setattr=iod_setattr,
 .getattr=iod_getattr,
 .setxattr=iod_setxattr,
 .getxattr=iod_getxattr,
 .listxattr=iod_listxattr,
 .removexattr=iod_removexattr,
};

