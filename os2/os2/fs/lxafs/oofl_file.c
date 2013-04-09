/* $Id: oofl_file.c,v 1.16 2005/06/21 23:04:44 smilcke Exp $ */

/*
 * oofl_file.c
 * Autor:               Stefan Milcke
 * Erstellt am:         04.02.2005
 * Letzte Aenderung am: 19.06.2005
 *
*/

#define LXAFS_DECLARATIONS
#include <lxcommon.h>

#include "oofl_fs.h"

/*******************************************************************************/
/* inode operations                                                           */
/*******************************************************************************/
/*
static int
 iof_create(struct inode* inode,struct dentry* dentry,int i,struct nameidata* nameidata)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static struct dentry*
 iof_lookup(struct inode* inode,struct dentry* dentry,struct nameidata* nameidata)
{
 const char* name=dentry->d_name.name;
 int len=dentry->d_name.len;
 LX_FUNC_LOG_ENTRY();
 printk("Requested d_entry name:%s\n",name);
 DevInt3();
 return 0;
}
*/

/*
static int
 iof_link(struct dentry* dentry,struct inode* inode,struct dentry* dentryt)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static int
 iof_unlink(struct inode* inode,struct dentry* dentry)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static int
 iof_symlink(struct inode* inode,struct dentry* dentry,const char* c)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static int
 iof_mkdir(struct inode* inode,struct dentry* dentry,int i)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static int
 iof_rmdir(struct inode* inode,struct dentry* dentry)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static int
 iof_mknod(struct inode* inode,struct dentry* dentry,int i,dev_t dev)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static int
 iof_rename(struct inode* inode1,struct dentry* dentry1,struct inode* inode2,struct dentry* dentry2)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static int
 iof_readlink(struct dentry* dentry, char __user* c,int i)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static int
 iof_follow_link(struct dentry* dentry,struct nameidata* nameidata)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

//-------------------------------- iof_truncate --------------------------------
static void
 iof_truncate(struct inode* inode)
{
 HFILE hFile=0;
 ULONG rc;
 struct lxafs_inode_info* li=LXAFS_I(inode);
 rc=lxafs_open_inode_for_maintenance(inode,&hFile);
 if(rc>=0)
 {
  rc=LX_DOSSETFILESIZE(hFile,inode->i_size);
  if(!rc)
   li->ia_valid&=~ATTR_SIZE;
  LX_DOSCLOSE(hFile);
  if(rc)
   mark_inode_dirty(inode);
 }
}

/*
static int
 iof_permission(struct inode* inode,int i,struct nameidata* nameidata)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return 0;
}
*/

//-------------------------------- iof_setattr ---------------------------------
static int
 iof_setattr(struct dentry* dentry,struct iattr* attr)
{
 struct inode* inode=dentry->d_inode;
 struct lxafs_inode_info* li=LXAFS_I(inode);
 int error=0;
 li->ia_valid|=attr->ia_valid;
 lock_kernel();
 error=inode_change_ok(inode,attr);
 if(!error)
  error=inode_setattr(inode,attr);
 unlock_kernel();
 if(!error)
  lxafs_write_inode(inode,1);
 return error;
}

/*
static int
 iof_getattr(struct vfsmount* mnt,struct dentry* dentry,struct kstat* kstat)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static int
 iof_setxattr(struct dentry* dentry,const char* c,const void* data,size_t len,int i)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static ssize_t
 iof_getxattr(struct dentry* dentry,const char* c,void* data,size_t len)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static ssize_t
 iof_listxattr(struct dentry* dentry,char* c,size_t len)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static int
 iof_removexattr(struct dentry* dentry,const char* c)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*******************************************************************************/
/* file operations                                                            */
/*******************************************************************************/
/*
static loff_t fof_llseek(struct file *file, loff_t offset, int origin)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

//---------------------------------- fof_read ----------------------------------
static ssize_t
 fof_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
 struct lxafs_filep_priv_f* pf;
 if(!buf)
  return -EFAULT;
 pf=file->private_data;
 if(pf)
 {
  if(pf->ulHandle)
  {
   ULONG cbBytes=count;
   ULONG offset;
   ULONG rc;
   if(ppos)
    offset=*ppos;
   else
    offset=file->f_pos;
   rc=SecHlpRead(pf->ulHandle,&cbBytes,buf,0,offset);
   if(rc)
    return -EBADF;
   if(ppos)
    (*ppos)+=cbBytes;
   return cbBytes;
  }
  else
   return -EBADF;
 }
 return -ENOENT;
}

/*
static ssize_t
 fof_aio_read(struct kiocb *iocb, char __user *buf, size_t count, loff_t pos)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

//--------------------------------- fof_write ----------------------------------
static ssize_t
 fof_write(struct file *filp, const char __user *buf,size_t count, loff_t *ppos)
{
 struct lxafs_filep_priv_f* pf;
 if(!buf)
  return -EFAULT;
 pf=filp->private_data;
 if(pf)
 {
  if(pf->ulHandle)
  {
   ULONG cbBytes=count;
   ULONG offset;
   ULONG rc;
   if(ppos)
    offset=*ppos;
   else
    offset=filp->f_pos;
   rc=SecHlpWrite(pf->ulHandle,&cbBytes,(char*)buf,0,offset);
   if(rc)
    return -EBADF;
   if(ppos)
    (*ppos)+=cbBytes;
   return cbBytes;
  }
  else
   return -EBADF;
 }
 return -ENOENT;
}

/*
static ssize_t
 fof_aio_write(struct kiocb *iocb, const char __user *buf,size_t count, loff_t pos)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

/*
static int
 fof_readdir(struct file * filp, void * dirent, filldir_t filldir)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

static unsigned int
 fof_poll(struct file *filp, struct poll_table_struct *wait)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

static int
 fof_ioctl(struct inode * inode, struct file * filp,unsigned int cmd, unsigned long arg)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

static int
 fof_mmap(struct file * file, struct vm_area_struct * vma)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

//---------------------------------- fof_open ----------------------------------
static int
 fof_open(struct inode * inode, struct file * filp)
{
 int rc;
 struct lxafs_filep_priv_f* pf;
 struct lxafs_inode_info* li=LXAFS_I(inode);
 pf=kmalloc(sizeof(struct lxafs_filep_priv_f),GFP_KERNEL);
 if((!pf))
  goto out_nomem;
 filp->private_data=pf;
 memset(pf,0,sizeof(struct lxafs_filep_priv_f));
 pf->ulOpenMode=OPEN_FLAGS_FAIL_ON_ERROR;
 switch((filp->f_flags&O_ACCMODE))
 {
  case O_RDONLY:
   pf->ulOpenMode=OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE;
   break;
  case O_WRONLY:
//   pf->ulOpenMode=OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYWRITE;
   pf->ulOpenMode=OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE;
   break;
  case O_RDWR:
//   pf->ulOpenMode=OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE;
   pf->ulOpenMode=OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE;
   break;
 }
 pf->ulOpenMode|=OPEN_FLAGS_FAIL_ON_ERROR;
 if(filp->f_flags&O_CREAT)
 {
  if(filp->f_flags&O_EXCL)
   pf->ulOpenFlags|=OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS;
  else
   pf->ulOpenFlags|=OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS;
 }
 else
  pf->ulOpenFlags|=OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS;
/*
 if(filp->f_flags&O_TRUNC)
 {
  pf->ulOpenFlags&=~OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_OPEN_IF_EXISTS);
  pf->ulOpenFlags|=OPEN_ACTION_REPLACE_IF_EXISTS;
 }
*/
 rc=SecHlpOpen(li->linkinfo.name,&pf->ulHandle,pf->ulOpenFlags,pf->ulOpenMode);
 if(rc || pf->ulHandle==0)
 {
  filp->private_data=0;
  rc=-EPERM;
  goto out_err;
 }
 {
  unsigned long sz;
  rc=SecHlpQFileSize(pf->ulHandle,&sz);
  if(!rc)
   inode->i_size=sz;
 }
 return 0;
out_nomem:
 rc=-ENOMEM;
out_err:
 if(pf)
  kfree(pf);
 return rc;
}

/*
static int
 fof_flush(struct file *file)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}
*/

//-------------------------------- fof_release ---------------------------------
static int
 fof_release(struct inode * inode, struct file * filp)
{
 struct lxafs_filep_priv_f* pf;
 pf=filp->private_data;
 if(pf)
 {
  if(pf->ulHandle)
  {
   SecHlpClose(pf->ulHandle);
   kfree(pf);
   filp->private_data=0;
   return 0;
  }
  else
   return -EBADF;
 }
 return -ENOENT;
}

static int
 fof_fsync(struct file * filp, struct dentry *dentry, int datasync)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

static int
 fof_fasync(int fd, struct file *filp, int on)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

static int
 fof_lock(struct file *filp, int cmd, struct file_lock *fl)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

static ssize_t
 fof_readv(struct file *filp, const struct iovec *vector,unsigned long count, loff_t *ppos)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

static ssize_t
 fof_writev(struct file *filp, const struct iovec *vector,unsigned long count, loff_t *ppos)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

static ssize_t
 fof_sendfile(struct file *in_filp, loff_t *ppos,size_t count, read_actor_t actor, void __user *target)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

static ssize_t
 fof_sendpage(struct file *filp, struct page *page,int offset, size_t size, loff_t *ppos, int more)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

static unsigned long
 fof_get_unmapped_area(struct file *filp, unsigned long addr
                       ,unsigned long len, unsigned long pgoff, unsigned long flags)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

static long
 fof_fcntl(int file_desc, unsigned int command, unsigned long arg,struct file* filp)
{
 LX_FUNC_LOG_ENTRY();
 DevInt3();
 return -ENOENT;
}

struct file_operations lxafs_file_fops=
{
 .owner=THIS_MODULE,
// .llseek=fof_llseek,
 .read=fof_read,
// .aio_read=fof_aio_read,
 .write=fof_write,
// .aio_write=fof_aio_write,
// .readdir=fof_readdir,
 .poll=fof_poll,
 .ioctl=fof_ioctl,
 .mmap=fof_mmap,
 .open=fof_open,
// .flush=fof_flush,
 .release=fof_release,
 .fsync=fof_fsync,
// .aio_fsync=fof_aio_fsync,
 .fasync=fof_fasync,
 .lock=fof_lock,
 .readv=fof_readv,
 .writev=fof_writev,
 .sendfile=fof_sendfile,
 .sendpage=fof_sendpage,
 .get_unmapped_area=fof_get_unmapped_area,
 .fcntl=fof_fcntl,
};

struct inode_operations lxafs_file_iops=
{
// .create=iof_create,
// .lookup=iof_lookup,
// .link=iof_link,
// .unlink=iof_unlink,
// .symlink=iof_symlink,
// .mkdir=iof_mkdir,
// .rmdir=iof_rmdir,
// .mknod=iof_mknod,
// .rename=iof_rename,
// .readlink=iof_readlink,
// .follow_link=iof_follow_link,
 .truncate=iof_truncate,
// .permission=iof_permission,
 .setattr=iof_setattr,
// .getattr=iof_getattr,
// .setxattr=iof_setxattr,
// .getxattr=iof_getxattr,
// .listxattr=iof_listxattr,
// .removexattr=iof_removexattr,
};
