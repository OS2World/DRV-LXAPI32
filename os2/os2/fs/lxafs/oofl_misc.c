/* $Id: oofl_misc.c,v 1.18 2005/07/25 20:04:25 smilcke Exp $ */

/*
 * oofl_misc.c
 * Autor:               Stefan Milcke
 * Erstellt am:         12.02.2005
 * Letzte Aenderung am: 24.07.2005
 *
*/

#define LXAFS_DECLARATIONS
#include <lxcommon.h>

#include "oofl_fs.h"

/*
fb->fdateCreation->day;
fb->fdateCreation->month;
fb->fdateCreation->year;  // since 1980
fb->ftimeCreation->twosecs;
fb->ftimeCreation->minutes;
fb->ftimeCreation->hours;

fb->fdateLastAccess

fb->fdateLastWrite
*/

extern struct timezone sys_tz;
static int lxafs_day_n[] = { 0,31,59,90,120,151,181,212,243,273,304,334,0,0,0,0 };
                         /* JanFebMarApr May Jun Jul Aug Sep Oct Nov Dec */

//----------------------------- lxafs_date_lx2unix -----------------------------
int lxafs_date_lx2unix(PFDATE pDate,PFTIME pTime)
{
 int year=pDate->year;
 int month=pDate->month;
 int day=pDate->day;
 int secs=sys_tz.tz_minuteswest*60;
 secs+=pTime->twosecs+pTime->minutes*60+pTime->hours*3600;
 secs+=86400*(pDate->day-1+lxafs_day_n[pDate->month]+(year/4)+year*365-
       ((year&3)==0 && month<2 ? 1 : 0)+3653);
 return secs;
}

//----------------------------- lxafs_date_unix2lx -----------------------------
void lxafs_date_unix2lx(int secs,PFDATE pDate,PFTIME pTime)
{
 int day,year,nl_day,month;
// unsigned short time;
// unsigned short date;
 secs-=sys_tz.tz_minuteswest*60;
 if(secs<315532800)
  secs=315532800;
 pTime->hours=(secs/3600)%24;
 pTime->minutes=(secs/60)%60;
 pTime->twosecs=(secs%60)/2;
// time=(secs%60)/2+(((secs/60)%60)<<5)+(((secs/3600)%24)<<11);
 day=secs/86400-3652;
 year=day/365;
 if((year+3)/4+365*year>day)
  year--;
 day-=(year+3)/4+365*year;
 if(day==59 && !(year & 3))
 {
  nl_day=day;
  month=2;
 }
 else
 {
  nl_day=(year&3)||day<=59 ? day : day-1;
  for(month=0;month<12;month++)
   if(lxafs_day_n[month]>nl_day)
    break;
 }
// date=nl_day-lxafs_day_n[month-1]+1+(month<<5)+(year<<9);
 pDate->day=nl_day-lxafs_day_n[month-1];
 pDate->month=month;
 pDate->year=year;
}

//----------------------------- lxafs_add_linkinfo -----------------------------
int lxafs_add_linkinfo(struct inode* inode,struct lxafs_linkinfo* linkinfo
                       ,int tail)
{
 int rc=0;
 struct list_head *lh;
 struct lxafs_inode_info* li=LXAFS_I(inode);
 struct lxafs_linkinfo* lfo;
 list_for_each(lh,&li->linkinfo.list)
 {
  lfo=list_entry(lh,struct lxafs_linkinfo,list);
  if(!strncmp(lfo->name,linkinfo->name,LX_MAXPATH))
  {
   rc=-EBUSY;
   break;
  }
 }
 if(!rc)
 {
  if(tail)
   list_add_tail(&linkinfo->list,&li->linkinfo.list);
  else
   list_add(&linkinfo->list,&li->linkinfo.list);
 }
 return rc;
}

//---------------------------- lxafs_alloc_linkinfo ----------------------------
struct lxafs_linkinfo* lxafs_alloc_linkinfo(int xname_offset)
{
 struct lxafs_linkinfo* li;
 li=kmalloc(sizeof(struct lxafs_linkinfo),GFP_KERNEL);
 if(li)
 {
  INIT_LIST_HEAD(&li->list);
  li->xname=li->name+xname_offset;
 }
 return li;
}

//---------------------------- lxafs_free_linkinfo -----------------------------
void lxafs_free_linkinfo(struct lxafs_linkinfo* linkinfo)
{
 if(linkinfo)
  kfree(linkinfo);
}

//---------------------------- lxafs_alloc_fileinfo ----------------------------
struct lxafs_fileinfo* lxafs_alloc_fileinfo(int xname_offset)
{
 struct lxafs_fileinfo* fi;
 fi=kmalloc(sizeof(struct lxafs_fileinfo),GFP_KERNEL);
 if(fi)
 {
  memset(fi,0,sizeof(struct lxafs_fileinfo));
  INIT_LIST_HEAD(&fi->linkinfo.list);
  fi->linkinfo.xname=fi->linkinfo.name+xname_offset;
 }
 return fi;
}

//---------------------------- lxafs_free_fileinfo -----------------------------
void lxafs_free_fileinfo(struct lxafs_fileinfo* fileinfo)
{
 if(fileinfo)
 {
  struct list_head *lh,*tmp;
  struct lxafs_linkinfo *lfo;
  list_for_each_safe(lh,tmp,&fileinfo->linkinfo.list)
  {
   lfo=list_entry(lh,struct lxafs_linkinfo,list);
   list_del(&lfo->list);
   lxafs_free_linkinfo(lfo);
  }
  kfree(fileinfo);
 }
}

//---------------------------- LX_os2path_from_file ----------------------------
int LX_os2path_from_file(struct file* file,char* buf,int buflen)
{
 struct super_block* sb=0;
 if(file->f_dentry->d_inode)
  sb=file->f_dentry->d_inode->i_sb;
 if(sb && !strncmp(sb->s_id,"lxda",4))
 {
  int c=0;
  struct lxafs_inode_info* li=LXAFS_I(file->f_dentry->d_inode);
  while(c!=buflen)
  {
   if(li->linkinfo.name[c]=='/')
    buf[c]='\\';
   else
    buf[c]=li->linkinfo.name[c];
   c++;
  }
  return 0;
 }
 else
 {
  char* fp;
  char* pf;
  DevInt3();
  fp=(char*)kmalloc(LX_MAXPATH,GFP_KERNEL);
  if(!fp)
   return -ENOMEM;
  pf=d_path(file->f_dentry,file->f_vfsmnt,fp,LX_MAXPATH);
  strncpy(fp,pf,LX_MAXPATH);
  strncpy(buf,fp,buflen);
  kfree(fp);
  return 0;
 }
}

extern struct list_head lxafs_sblist;

//------------------------------ LX_os2path_to_lx ------------------------------
int LX_os2path_to_lx(const char* os2path,char* lxpath)
{
 int i;
 int olen;
 char opath[LX_MAXPATH];
 struct list_head* lh;
 struct lxafs_super_info* sbi;
 memset(opath,0,LX_MAXPATH);
 olen=strnlen(os2path,LX_MAXPATH);
 for(i=0;i<olen;i++)
 {
  if(os2path[i]=='\\')
   opath[i]='/';
  else
   opath[i]=os2path[i];
 }
 list_for_each(lh,&lxafs_sblist)
 {
  sbi=list_entry(lh,struct lxafs_super_info,list);
  if(!strnicmp(sbi->rootpath,opath,strlen(sbi->rootpath)))
  {
   strncpy(lxpath,sbi->xrootpath,LX_MAXPATH);
   strncat(lxpath,&(opath[strlen(sbi->rootpath)+1]),LX_MAXPATH-strlen(lxpath));
   return 0;
  }
 }
 return -ENOENT;
}
