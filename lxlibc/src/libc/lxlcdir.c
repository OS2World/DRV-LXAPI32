/* $Id: lxlcdir.c,v 1.9 2005/06/21 23:04:36 smilcke Exp $ */

/*
 * lxlcdir.c
 * Autor:               Stefan Milcke
 * Erstellt am:         30.04.2005
 * Letzte Aenderung am: 20.06.2005
 *
*/

#define INCL_DOS
#define LXFH_MAP
#include <lxlibc.h>

#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <stdarg.h>
#include <share.h>
#include <sys/stat.h>

#include <dirent.h>

#include <lxlibcwcalls.h>

#include <linux/err.h>
#include <linux/unistd.h>
#include <lxlist.h>

#include <lxlibcsysc.h>

#define LXMAXPATH    (300)

extern int maybe_unix_file(__const__ char* name);

//------------------------------ LXAPI_os_unlink -------------------------------
LXWCALLLINKAGE int LXWCATTRINLINE
  LXAPI_os_unlink(char* name)
{
 int rc;
 __asm__ __volatile__
 (
  "call  _unlink\n\t"
  :"=a" ((int)rc)
  :"a" ((unsigned long)name)
  :"ecx", "edx", "memory"
 );
 return rc;
}

//------------------------------ LXAPI_os_rename -------------------------------
LXWCALLLINKAGE int LXWCATTRINLINE
  LXAPI_os_rename(__const__ char* oldname,__const__ char* newname)
{
 int rc;
 __asm__ __volatile__
 (
  "call  rename\n\t"
  : "=a" ((int)rc)
  : "a" ((unsigned long)oldname), "d" ((unsigned long) newname)
  : "ecx", "esi", "edi", "memory"
 );
 return rc;
}

//------------------------------- LXAPI_os_chmod -------------------------------
LXWCALLLINKAGE int LXWCATTRINLINE
  LXAPI_os_chmod(__const__ char* name,int pmode)
{
 int rc;
 __asm__ __volatile__
 (
  "call  _chmod\n\t"
  : "=a" ((int)rc)
  : "a" ((unsigned long)name), "d" ((unsigned long) pmode)
  : "ecx", "esi", "edi", "memory"
 );
 return rc;
}

//-------------------------------- LXAPI_mkdir ---------------------------------
int LXAPI_mkdir(__const__ char* dirname,mode_t mode)
{
 int rc;
 rc=SYSCALL2(__NR_mkdir,dirname,mode);
 return rc;
}

//-------------------------------- LXAPI_rmdir ---------------------------------
int LXAPI_rmdir(__const__ char* dirname)
{
 int rc;
 rc=SYSCALL1(__NR_rmdir,dirname);
 return rc;
}

//--------------------------------- LXAPI_link ---------------------------------
int LXAPI_link(__const__ char* oldname,__const__ char* newname)
{
 int rc;
 if(!maybe_unix_file(oldname) || !maybe_unix_file(newname))
  return -1;
 rc=SYSCALL2(__NR_link,oldname,newname);
 return rc;
}

//-------------------------------- LXAPI_unlink --------------------------------
int LXAPI_unlink(__const__ char* name)
{
 int rc;
 if(maybe_unix_file(name))
 {
  rc=SYSCALL1(__NR_unlink,name);
  if(rc)
   rc=LXAPI_os_unlink((char*)name);
 }
 else
  rc=LXAPI_os_unlink((char*)name);
 return rc;
}

//------------------------------- LXAPI_symlink --------------------------------
int LXAPI_symlink(__const__ char* oldname,__const__ char* newname)
{
 int rc;
 rc=SYSCALL2(__NR_symlink,oldname,newname);
 return rc;
}

//-------------------------------- LXAPI_getcwd --------------------------------
char* LXAPI_getcwd(char* dirname,size_t buflen)
{
 int rc;
 rc=SYSCALL2(__NR_getcwd,dirname,buflen);
 if(rc)
  return NULL;
 return dirname;
}

//-------------------------------- LXAPI_chdir ---------------------------------
int LXAPI_chdir(__const__ char* dirname)
{
 int rc;
 rc=SYSCALL1(__NR_chdir,dirname);
 return rc;
}

//-------------------------------- LXAPI_rename --------------------------------
int LXAPI_rename(__const__ char* oldname,__const__ char* newname)
{
 int rc;
 if(maybe_unix_file(oldname) && maybe_unix_file(newname))
 {
  rc=SYSCALL2(__NR_rename,oldname,newname);
  if(rc)
   rc=LXAPI_os_rename(oldname,newname);
 }
 else
  rc=LXAPI_os_rename(oldname,newname);
 return rc;
}

//-------------------------------- LXAPI_chroot --------------------------------
int LXAPI_chroot(__const__ char* path)
{
 int rc;
 rc=SYSCALL1(__NR_chroot,path);
 return rc;
}

//-------------------------------- LXAPI_chmod ---------------------------------
int LXAPI_chmod(__const__ char* name,int pmode)
{
 int rc=0;
 if(maybe_unix_file(name))
  rc=SYSCALL2(__NR_chmod,name,pmode);
 else
  rc=1;
 if(rc)
  rc=LXAPI_os_chmod(name,pmode);
 return rc;
}

//----------------------------------- fchmod -----------------------------------
int fchmod(int fd,mode_t mode)
{
 struct lx_filehandle* fhp=LXAPI_get_filehandle_struct(fd);
 if(fhp)
  return SYSCALL2(__NR_fchmod,fhp->real_handle,mode);
 else
  return -1;
}

//--------------------------------- ftruncate ----------------------------------
int ftruncate(int fd,off_t offset)
{
 struct lx_filehandle* fhp=LXAPI_get_filehandle_struct(fd);
 if(fhp)
  return SYSCALL2(__NR_ftruncate,fhp->real_handle,offset);
 else
  return -1;
}

//---------------------------------- readdir -----------------------------------
int readdir(unsigned int fd,struct old_linux_dirent *dirent,unsigned int count)
{
 int rc;
 struct lx_filehandle* fhp=LXAPI_get_filehandle_struct(fd);
 if(fhp)
  rc=SYSCALL3(__NR_readdir,fhp->real_handle,dirent,count);
 else
  rc=-1;
 return rc;
}

//---------------------------------- getdents ----------------------------------
int getdents(unsigned int fd,struct linux_dirent *dirent,unsigned int count)
{
 int rc;
 struct lx_filehandle* fhp=LXAPI_get_filehandle_struct(fd);
 if(fhp)
  rc=SYSCALL3(__NR_getdents,fhp->real_handle,dirent,count);
 else
  rc=-1;
 return rc;
}
