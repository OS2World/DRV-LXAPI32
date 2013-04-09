/* $Id: oofl_io.c,v 1.23 2005/06/28 22:00:55 smilcke Exp $ */

/*
 * oofl_io.c
 * Autor:               Stefan Milcke
 * Erstellt am:         11.02.2005
 * Letzte Aenderung am: 28.06.2005
 *
*/

#define LXAFS_DECLARATIONS
#include <lxcommon.h>

#include "oofl_fs.h"

//--------------------------- lxafs_init_inode_type ----------------------------
static int lxafs_init_inode_type(struct inode* inode,struct lxafs_fileinfo* fileinfo)
{
 inode->i_mode=0;
 if(fileinfo->fs4.attrFile&FILE_DIRECTORY)
 {
  inode->i_mode=S_IRWXUGO | S_IFDIR;
  inode->i_nlink=fileinfo->num_subdirs;
 }
 else
 {
  inode->i_mode=S_IRWXUGO | S_IFREG;
  inode->i_nlink=1;
 }
 if(fileinfo->flags&LXGETFILEINFOFLAG_MODE)
  inode->i_mode=fileinfo->mode;
 else
  inode->i_mode|=0111;
 if(fileinfo->flags&LXGETFILEINFOFLAG_UID)
  inode->i_uid=fileinfo->uid;
 if(fileinfo->flags&LXGETFILEINFOFLAG_GID)
  inode->i_gid=fileinfo->gid;
 inode->i_ctime.tv_sec=lxafs_date_lx2unix(&fileinfo->fs4.fdateCreation
                                          ,&fileinfo->fs4.ftimeCreation);
 inode->i_mtime.tv_sec=lxafs_date_lx2unix(&fileinfo->fs4.fdateLastWrite
                                          ,&fileinfo->fs4.ftimeLastWrite);
 inode->i_atime.tv_sec=lxafs_date_lx2unix(&fileinfo->fs4.fdateLastAccess
                                          ,&fileinfo->fs4.ftimeLastAccess);
 inode->i_size_seqcount.sequence=0;
 inode->i_size=fileinfo->fs4.cbFile;
 return 0;
}

//------------------------------ lxafs_init_inode ------------------------------
int lxafs_init_inode(struct inode* i,struct lxafs_fileinfo* fileinfo)
{
 int rc;
 struct super_block* sb=i->i_sb;
 struct lxafs_fileinfo* fi=0;
 i->i_blksize=512;
 i->i_size=-1;
 i->i_blocks=-1;
 i->i_ctime.tv_sec = i->i_ctime.tv_nsec = 0;
 i->i_mtime.tv_sec = i->i_mtime.tv_nsec = 0;
 i->i_atime.tv_sec = i->i_atime.tv_nsec = 0;
 if(!fileinfo)
  return -ENOENT;
 rc=lxafs_init_inode_type(i,fileinfo);
 if(S_ISDIR(i->i_mode))
 {
  i->i_op=&lxafs_dir_iops;
  i->i_fop=&lxafs_dir_fops;
 }
 else
 {
  i->i_op=&lxafs_file_iops;
  i->i_fop=&lxafs_file_fops;
  i->i_nlink=1;
 }
 return rc;
}

//---------------------------- lxafs_do_read_inode -----------------------------
int lxafs_do_read_inode(struct inode* inode,char* fullname)
{
 int rc;
 struct lxafs_fileinfo* fileinfo;
 fileinfo=lxafs_get_fullfileinfo(fullname,LXAFS_ROOTPATH_I(inode));
 if(fileinfo)
 {
  lxafs_init_inode(inode,fileinfo);
  lxafs_free_fileinfo(fileinfo);
 }
 return rc;
}

//----------------------------- lxafs_create_file ------------------------------
int lxafs_create_file(char* filename)
{
 int ret;
 unsigned long handle=0;
 ret=SecHlpOpen(filename,&handle
                ,OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_FAIL_IF_EXISTS
                ,OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYWRITE | OPEN_FLAGS_FAIL_ON_ERROR);
 if(!ret && handle!=0)
  SecHlpClose(handle);
 return ret;
}

//---------------------- lxafs_open_inode_for_maintenance ----------------------
int lxafs_open_inode_for_maintenance(struct inode* inode,HFILE* pHFile)
{
 struct lxafs_inode_info* li=LXAFS_I(inode);
 if(lxafs_is_lxafs_inode(inode))
 {
  ULONG ulAction;
  int rc;
  rc=LX_DOSOPEN(li->linkinfo.name,pHFile,&ulAction,inode->i_size,FILE_NORMAL
                ,OPEN_ACTION_CREATE_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS
                ,OPEN_FLAGS_FAIL_ON_ERROR|OPEN_SHARE_DENYREADWRITE|OPEN_ACCESS_READWRITE
                ,0);
  if(rc)
   return -EPERM;
  else
   return 0;
 }
 return -ENOENT;
}

//----------------------------- lxafs_get_fileinfo -----------------------------
int lxafs_get_fileinfo(char* name,void* buf,unsigned long ulInfoLevel)
{
 unsigned long rc;
 unsigned long sz;
 rc=LX_DOSQUERYPATHINFO(name,ulInfoLevel,buf,sizeof(FILESTATUS4));
 if(rc==ERROR_FILE_NOT_FOUND || rc==ERROR_PATH_NOT_FOUND)
  return -ENOENT;
 if(rc)
 {
  FILEFINDBUF4 fb;
  HDIR hDir=HDIR_CREATE;
  ULONG cFileNames=1;
  rc=LX_DOSFINDFIRST(name,&hDir
                     ,FILE_DIRECTORY|FILE_HIDDEN|FILE_SYSTEM|FILE_NORMAL
                     ,&fb,sizeof(FILEFINDBUF4),&cFileNames,ulInfoLevel);
  if(!rc)
  {
   LX_DOSFINDCLOSE(hDir);
   if(!cFileNames)
    rc=-ENOENT;
   else
    memcpy(buf,&fb.fdateCreation,sizeof(FILESTATUS4));
  }
  else
   rc=-ENOENT;
 }
 return rc;
}

//--------------------------- lxafs_get_fullfileinfo ---------------------------
struct lxafs_fileinfo* lxafs_get_fullfileinfo(char* name,char* rootpath)
{
 struct lxafs_fileinfo* fileinfo;
 int rc;
 int lc=0;
 int rpl;
 ULONG blen;
 char tmp[100];
 LXGETFILEINFOSTRUCT fs;
 rpl=strlen(rootpath);
 fileinfo=lxafs_alloc_fileinfo(rpl);
 if(!fileinfo)
  return 0;
/*
 rc=LX_GETFILEINFO(name,fileinfo->linkinfo.name,&fileinfo->fs4
                   ,&fileinfo->num_subdirs,&num_files,&lc,&link_broken);
*/
 rc=LX_GETFILEINFO(name,&fs);
 if(rc)
  goto fail;
 strncpy(fileinfo->linkinfo.name,fs.linkName,LX_MAXPATH);
 memcpy(&(fileinfo->fs4),&(fs.fs4),sizeof(FILESTATUS4));
 fileinfo->num_subdirs=fs.num_subdirs;
 fileinfo->num_files=fs.num_files;
 fileinfo->mode=fs.mode;
 fileinfo->uid=fs.uid;
 fileinfo->gid=fs.gid;
 fileinfo->symlink=fs.symlink;
 fileinfo->flags=fs.flags;
 lc=fs.lc_count;
 while(lc)
 {
  struct lxafs_linkinfo* linkinfo;
  linkinfo=lxafs_alloc_linkinfo(rpl);
  if(!linkinfo)
   goto fail;
  sprintf(tmp,LXEAN_LINKEDBY,lc);
  blen=LX_MAXPATH;
  LX_EAREADSTRING(name,tmp,linkinfo->name,&blen);
  if(strlen(linkinfo->name)>0)
   list_add(&linkinfo->list,&fileinfo->linkinfo.list);
  else
   lxafs_free_linkinfo(linkinfo);
  lc--;
 }
 return fileinfo;
fail:
 lxafs_free_fileinfo(fileinfo);
 return NULL;
}

//----------------------------- lxafs_file_exists ------------------------------
int lxafs_file_exists(char* fname)
{
 int rc;
 FILESTATUS4 fs4;
 rc=lxafs_get_fileinfo(fname,&fs4,FIL_QUERYEASIZE);
 if(rc)
  return 0;
 else
  return 1;
}

//--------------------------- lxafs_rewrite_linklist ---------------------------
int lxafs_rewrite_linklist(struct inode* inode)
{
 struct lxafs_inode_info* li=LXAFS_I(inode);
 int rc=-ENOMEM;
 int i;
 char *name;
 char *val;
 ULONG blen;
 struct list_head* lh;
 struct lxafs_linkinfo* lfo;
 if(!lxafs_is_lxafs_inode(inode))
  return 0;
 rc=LX_alloc_objects(2,LX_MAXPATH,GFP_KERNEL,&name,&val);
 if(rc)
  return rc;
 val[0]=(char)0;
 strcpy(name,LXEAN_LINKBYCOUNT);
 blen=LX_MAXPATH;
 LX_EAREADSTRING(li->linkinfo.name,name,val,&blen);
 i=simple_strtoul(val,0,0);
 LX_EADELETE(li->linkinfo.name,LXEAN_LINKBYCOUNT);
 while(i)
 {
  sprintf(name,LXEAN_LINKEDBY,i);
  LX_EADELETE(li->linkinfo.name,name);
  i--;
 }
 list_for_each(lh,&li->linkinfo.list)
 {
  lfo=list_entry(lh,struct lxafs_linkinfo,list);
  i++;
  sprintf(name,LXEAN_LINKEDBY,i);
  strncpy(val,lfo->name,LX_MAXPATH);
  blen=strlen(val)+1;
  LX_EAWRITESTRING(li->linkinfo.name,name,val,&blen);
  if(li->dirty_flag&LXAFS_DIRTY_LINKCHILDS)
   lxafs_write_linkchild_info(lfo->name,li->linkinfo.name);
 }
 if(i)
 {
  sprintf(val,"%d",i);
  blen=strlen(val)+1;
  LX_EAWRITESTRING(li->linkinfo.name,LXEAN_LINKBYCOUNT,val,&blen);
 }
out:
 kfree(name);
 return rc;
}

//-------------------------- lxafs_attach_linked_file --------------------------
int lxafs_attach_linked_file(struct lxafs_linkinfo* linkmaster
                              ,struct lxafs_linkinfo* link)
{
 int rc=0;
 struct nameidata nd;
 rc=path_lookup(linkmaster->xname,LOOKUP_ACCESS,&nd);
 if(!rc)
 {
  if(nd.dentry && nd.dentry->d_inode)
  {
   struct inode* inode=nd.dentry->d_inode;
   struct lxafs_inode_info* li=LXAFS_I(inode);
   struct lxafs_linkinfo* lifo;
   lifo=lxafs_alloc_linkinfo(strlen(LXAFS_ROOTPATH_I(inode)));
   if(lifo)
   {
    strncpy(lifo->name,link->name,LX_MAXPATH);
    if(lxafs_add_linkinfo(inode,lifo,0))
     kfree(lifo);
    else
     lxafs_mark_inode_dirty(inode,LXAFS_DIRTY_LINKLIST);
//    lxafs_rewrite_linklist(inode);
   }
  }
  path_release(&nd);
 }
 return rc;
}

//-------------------------- lxafs_detach_linked_file --------------------------
int lxafs_detach_linked_file(struct lxafs_linkinfo* linkmaster
                             ,struct lxafs_linkinfo* link)
{
 int rc=0;
 struct nameidata nd;
 rc=path_lookup(linkmaster->xname,LOOKUP_ACCESS,&nd);
 if(!rc)
 {
  rc=-ENOENT;
  if(nd.dentry && nd.dentry->d_inode)
  {
   struct inode* inode=nd.dentry->d_inode;
   struct lxafs_inode_info* li=LXAFS_I(inode);
   struct list_head* lh;
   struct lxafs_linkinfo* lfo;
   list_for_each(lh,&li->linkinfo.list)
   {
    lfo=list_entry(lh,struct lxafs_linkinfo,list);
    if(!strncmp(lfo->name,link->name,LX_MAXPATH))
    {
     list_del(&lfo->list);
     lxafs_free_linkinfo(lfo);
     lxafs_mark_inode_dirty(inode,LXAFS_DIRTY_LINKLIST);
     rc=0;
     break;
    }
   }
  }
 }
 return rc;
}

static char
 lxafs_dummy_msg[]="# This is a dummy file representing a %s." \
                   " Do not edit contents or EAs!\n"
                   "# S:%s\n";

//------------------------- lxafs_write_linkchild_info -------------------------
int lxafs_write_linkchild_info(char* fname,char* linkmaster)
{
 ULONG vallen;
 ULONG handle;
 if(!SecHlpOpen(fname,&handle
                ,OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS
                ,OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYWRITE | OPEN_FLAGS_FAIL_ON_ERROR))
 {
  char* tmp=kmalloc(LX_MAXPATH*2+strlen(lxafs_dummy_msg)+2,GFP_KERNEL);
  if(tmp)
  {
   sprintf(tmp,lxafs_dummy_msg,"link",linkmaster);
   vallen=strlen(tmp);
   SecHlpWrite(handle,&vallen,tmp,0,0);
   kfree(tmp);
  }
  SecHlpClose(handle);
  vallen=strlen(linkmaster)+1;
  LX_EAWRITESTRING(fname,LXEAN_LINKEDTO,linkmaster,&vallen);
  return 0;
 }
 else
  return 1;
}

