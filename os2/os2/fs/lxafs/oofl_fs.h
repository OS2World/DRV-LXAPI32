/* $Id: oofl_fs.h,v 1.24 2005/07/25 20:04:25 smilcke Exp $ */

/*
 * oofl_fs.h
 * Autor:               Stefan Milcke
 * Erstellt am:         20.04.2005
 * Letzte Aenderung am: 24.07.2005
 *
*/

#ifndef OOFL_FS_H_INCLUDED
#define OOFL_FS_H_INCLUDED

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/dcache.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/pagemap.h>
#include <linux/backing-dev.h>
#include <linux/namei.h>
#include <linux/smp_lock.h>
#include <linux/spinlock.h>
#include <linux/mount.h>

#include <lxfs.h>

#include <lxdaemon.h>

#include <lxsecure.h>

#ifdef PSIZE
#undef PSIZE
#endif
#define PSIZE  4096

#define ROOT_I 2  /* fileset root inode */

#define LXAFS_SUPER_MAGIC  0x28177128

#define LX_FUNC_LOG_ENTRY() {printk("Function %s{%s} entered\n",__FUNCTION__,__FILE__);}
#define LX_FUNC_LOG_EXIT() {printk("Function %s{%s} leaved\n",__FUNCTION__,__FILE__);}

extern struct file_operations    lxafs_file_fops;
extern struct file_operations    lxafs_dir_fops;
extern struct inode_operations   lxafs_file_iops;
extern struct inode_operations   lxafs_dir_iops;
extern struct super_operations   lxafs_super_operations;
extern struct export_operations  lxafs_export_operations;

struct lxafs_linkinfo
{
 struct  list_head list;
 char    name[LX_MAXPATH];
 char*   xname;
};

struct lxafs_fileinfo
{
 FILESTATUS4 fs4;
 int num_subdirs;
 int num_files;
 int mode;
 int uid;
 int gid;
 int symlink;
 int flags;
 struct lxafs_linkinfo linkinfo; // If linkedStd and linkedX are empty, this is the
                                 // master. ->list contains all links
};

#define LXAFS_DIRTY_LINKLIST     1
#define LXAFS_DIRTY_LINKCHILDS   2
struct lxafs_inode_info
{
 struct        inode inode;
 unsigned int  ia_valid;
 struct lxafs_linkinfo linkinfo; // linkedStd and linkedX contains file name of the
                                 // master. ->list contains all links
 int dirty_flag;
 struct list_head list;          // To hold in on SB's inode_list
};

struct lxafs_super_info
{
 struct  super_block* sb;
 char    rootpath[LX_MAXPATH];
 char    xrootpath[LX_MAXPATH];
 struct list_head inode_list;
 spinlock_t inode_list_lock;
 struct list_head list;
};

struct lxafs_filep_priv_f
{
 ULONG ulHandle;
 ULONG ulOpenFlags;
 ULONG ulOpenMode;
};

#define LXAFS_MAX_DENTS       (65536)
#define LXAFS_INO_UNDEFINED   (-1)
#define LXAFS_INO_UNKNOWN     (0)

struct lxafs_dirent
{
 unsigned long ino;
 unsigned int  type;
 char          fullname[LX_MAXPATH];
 char*         xname;
 char*         name;
};

struct lxafs_filep_priv_d
{
 unsigned long          num_ents;
 struct lxafs_dirent*   dirinfo;
 struct lxafs_dirent*   dirents;
 struct lxafs_fileinfo* old_dirinfo;
 struct lxafs_fileinfo** old_dirents;
};

static inline struct lxafs_filep_private* LXAFS_FPP(struct file* file)
{
 return (struct lxafs_filep_private*)file->private_data;
}

static inline struct lxafs_inode_info* LXAFS_I(struct inode* inode)
{
 return container_of(inode,struct lxafs_inode_info,inode);
}

static inline char* LXAFS_I_STDNAME(struct inode* inode)
{
 return LXAFS_I(inode)->linkinfo.name;
}

static inline char* LXAFS_I_XNAME(struct inode* inode)
{
 return LXAFS_I(inode)->linkinfo.xname;
}

static inline struct lxafs_super_info* LXAFS_SBI(struct super_block* super)
{
 return super->s_fs_info;
}

static inline char* LXAFS_ROOTPATH_I(struct inode* inode)
{
 return LXAFS_SBI(inode->i_sb)->rootpath;
}

static inline char* LXAFS_XROOTPATH_I(struct inode* inode)
{
 return LXAFS_SBI(inode->i_sb)->xrootpath;
}

static inline int lxafs_drive_no_from_super(struct super_block* sb)
{
 if(sb->s_id[5])
  return (int)((sb->s_id[4]-'1')*10+sb->s_id[5]-'1');
 else
  return (int)(sb->s_id[4]-'1');
}

static inline int lxafs_drive_no_from_inode(struct inode* inode)
{
 return lxafs_drive_no_from_super(inode->i_sb);
}

static inline int lxafs_is_lxafs_inode(struct inode* inode)
{
 if((inode->i_sb->s_id[0]=='l')
    && (inode->i_sb->s_id[1]=='x')
    && (inode->i_sb->s_id[2]=='d'))
  return 1;
 return 0;
}


// oofl_dentry.c
extern void lxafs_set_dentry_operations(struct dentry* dentry);

// oofl_name.c
extern unsigned char lxafs_upcase(unsigned char a);
extern void lxafs_adjust_length(unsigned char* name,unsigned* len);
extern int lxafs_chk_name(unsigned char* name,unsigned* len);
extern int lxafs_compare_names(struct super_block* sb
                               ,unsigned char* n1,unsigned l1
                               ,unsigned char* n2,unsigned l2
                               ,int last);
extern void lxafs_get_root_path_d(struct dentry* dentry,char* dir);
extern void lxafs_get_root_path_i(struct inode* inode,char* dir);
extern void lxafs_get_root_path_s(struct super_block* sb,char* dir);
#define LXAFS_BUILDFNAME_S    1
#define LXAFS_BUILDFNAME_X    2
extern int lxafs_build_fullname_d(char* buffer,int buflen,struct dentry* dentry,int flags);
extern int lxafs_build_fullname_i(char* buffer,int buflen,struct inode* inode,int flags);
extern void lxafs_build_fullname_from_parent_i(char* buf,struct inode* parent,char* name
                                               ,int flags);
extern void lxafs_build_fullname_from_parent_d(char* buf,struct dentry* parent,char* name
                                               ,int flags);

// oofl_io.c
extern int lxafs_create_file(char* filename);
extern int lxafs_rewrite_linklist(struct inode* inode);
extern int lxafs_attach_linked_file(struct lxafs_linkinfo* linkmaster
                                    ,struct lxafs_linkinfo* link);
extern int lxafs_detach_linked_file(struct lxafs_linkinfo* linkmaster
                                    ,struct lxafs_linkinfo* link);
extern int lxafs_open_inode_for_maintenance(struct inode* inode,HFILE* pHFile);
extern int lxafs_get_fileinfo(char* name,void* buf,unsigned long ulInfoLevel);
extern struct lxafs_fileinfo* lxafs_get_fullfileinfo(char* name,char* rootpath);
extern int lxafs_file_exists(char* fname);
extern int lxafs_do_read_inode(struct inode* inode,char* fullname);
extern int lxafs_write_linkchild_info(char* fname,char* linkmaster);

// oofl_misc.c
extern int lxafs_add_linkinfo(struct inode* inode,struct lxafs_linkinfo* linkinfo,int tail);
extern int lxafs_mark_inode_dirty(struct inode* inode,int flags);
extern int lxafs_date_lx2unix(PFDATE pDate,PFTIME pTime);
extern void lxafs_date_unix2lx(int secs,PFDATE pDate,PFTIME pTime);
extern struct lxafs_linkinfo* lxafs_alloc_linkinfo(int xname_offset);
extern void lxafs_free_linkinfo(struct lxafs_linkinfo* linkinfo);
extern struct lxafs_fileinfo* lxafs_alloc_fileinfo(int xname_offset);
extern void lxafs_free_fileinfo(struct lxafs_fileinfo* fileinfo);

// oofl_inode.c
extern int lxafs_init_inode(struct inode* i,struct lxafs_fileinfo* fileinfo);
extern void lxafs_write_inode(struct inode* inode,int sync);

#endif //OOFL_FS_H_INCLUDED
