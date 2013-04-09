/* $Id: oofl_name.c,v 1.10 2005/06/02 21:56:57 smilcke Exp $ */

/*
 * oofl_name.c
 * Autor:               Stefan Milcke
 * Erstellt am:         10.02.2005
 * Letzte Aenderung am: 02.06.2005
 *
*/

#define LXAFS_DECLARATIONS
#include <lxcommon.h>

#include "oofl_fs.h"

#include <linux/mount.h>

static inline int not_allowed_char(unsigned char c)
{
 return c<' ' || c=='"' || c=='*' || c=='/' || c==':' || c=='<' ||
        c=='>' || c=='?' || c=='\\' || c=='|';
}

//-------------------------------- lxafs_upcase --------------------------------
unsigned char lxafs_upcase(unsigned char a)
{
 if(a>='a' && a<='z')
  return a-'a'+'A';
 return a;
}

//---------------------------- lxafs_adjust_length -----------------------------
void lxafs_adjust_length(unsigned char *name,unsigned* len)
{
 if(!*len) return;
 if(*len==1 && name[0]=='.') return;
 if(*len==2 && name[0]=='.' && name[1]=='.') return;
 while(*len && (name[*len-1]=='.' || name[*len-1]==' '))
  (*len)--;
}

//------------------------------- lxafs_chk_name -------------------------------
int lxafs_chk_name(unsigned char* name,unsigned* len)
{
 int i;
 if(*len>254) return -ENAMETOOLONG;
 lxafs_adjust_length(name,len);
 if(!*len) return -EINVAL;
 for(i=0;i<*len;i++)
  if(not_allowed_char(name[i]))
   return -EINVAL;
 if(*len==1)
  if(name[0]=='.')
   return -EINVAL;
 if(*len==2)
  if(name[0]=='.' && name[1]=='.')
   return -EINVAL;
 return 0;
}

//---------------------------- lxafs_compare_names -----------------------------
int lxafs_compare_names(struct super_block *s, unsigned char *n1, unsigned l1
                        ,unsigned char *n2, unsigned l2, int last)
{
 unsigned l = l1 < l2 ? l1 : l2;
 unsigned i;
 if(last)
  return -1;
 for(i = 0; i < l; i++)
 {
  unsigned char c1 = lxafs_upcase(n1[i]);
  unsigned char c2 = lxafs_upcase(n2[i]);
  if(c1 < c2)
   return -1;
  if(c1 > c2)
   return 1;
 }
 if(l1 < l2)
  return -1;
 if(l1 > l2)
  return 1;
 return 0;
}

//--------------------- lxafs_build_fullname_from_parent_i ---------------------
void lxafs_build_fullname_from_parent_i(char* buf,struct inode* parent,char* name
                                        ,int flags)
{
 char pname[LX_MAXPATH]="";
 char sl[2]={0};
 if((flags&LXAFS_BUILDFNAME_S))
  sl[0]='\\';
 else
  sl[0]='/';
 if(lxafs_is_lxafs_inode(parent))
 {
  struct lxafs_inode_info* li=LXAFS_I(parent);
  if((flags&LXAFS_BUILDFNAME_S))
   strncpy(pname,li->linkinfo.name,LX_MAXPATH);
  else
   strncpy(pname,li->linkinfo.xname,LX_MAXPATH);
 }
 else
 {
  lxafs_build_fullname_i(pname,LX_MAXPATH,parent,flags);
 }
 strcpy(buf,pname);
 if(strlen(pname) && pname[strlen(pname)-1]!='/')
  strcat(buf,"/");
 strcat(buf,name);
}

//--------------------- lxafs_build_fullname_from_parent_d ---------------------
void lxafs_build_fullname_from_parent_d(char* buf,struct dentry* parent,char* name
                                        ,int flags)
{
 return lxafs_build_fullname_from_parent_i(buf,parent->d_inode,name,flags);
}

//--------------------------- lxafs_get_root_path_d ----------------------------
void lxafs_get_root_path_d(struct dentry* dentry,char* dir)
{
 lxafs_get_root_path_i(dentry->d_inode,dir);
}

//--------------------------- lxafs_get_root_path_i ----------------------------
void lxafs_get_root_path_i(struct inode* inode,char* dir)
{
 lxafs_get_root_path_s(inode->i_sb,dir);
}

//--------------------------- lxafs_get_root_path_s ----------------------------
void lxafs_get_root_path_s(struct super_block* sb,char* dir)
{
 int drive_no=lxafs_drive_no_from_super(sb);
 if(drive_no)
 {
  dir[0]=(char)(drive_no-1+'A');
  dir[1]=(char)':';
  dir[2]=(char)0;
 }
 else
 {
  int c=0;
  while(c<LX_MAXPATH)
  {
   if(lx_root_path[c]=='\\')
    dir[c]='/';
   else
    dir[c]=lx_root_path[c];
   c++;
  }
  if(dir[strlen(dir)-1]=='/')
   dir[strlen(dir)-1]=(char)0;
 }
}

//--------------------------- lxafs_build_fullname_i ---------------------------
int lxafs_build_fullname_i(char* buffer,int buflen,struct inode* inode,int flags)
{
 if(lxafs_is_lxafs_inode(inode))
 {
  struct lxafs_inode_info* li=LXAFS_I(inode);
  if(flags&LXAFS_BUILDFNAME_S)
   strncpy(buffer,li->linkinfo.name,buflen);
  else
   strncpy(buffer,li->linkinfo.xname,buflen);
 }
 return -ENOENT;
}

//--------------------------- lxafs_build_fullname_d ---------------------------
int lxafs_build_fullname_d(char* buffer,int buflen,struct dentry* dentry,int flags)
{
 return lxafs_build_fullname_i(buffer,buflen,dentry->d_inode,flags);
}
