/* $Id: lxlibc.h,v 1.24 2006/04/06 21:17:43 smilcke Exp $ */

/*
 * lxlibc.h
 * Autor:               Stefan Milcke
 * Erstellt am:         03.03.2005
 * Letzte Aenderung am: 21.07.2005
 *
*/

#ifndef LXLIBC_H_INCLUDED
#define LXLIBC_H_INCLUDED

#include <lxlibcdefs.h>

#ifndef NO_INCL_LXUTL_IOCTL
#ifndef LX_MAXPATH
#define LX_MAXPATH	260
#endif
#include <lxuapioctl.h>
#endif

#ifdef INCL_LXUTL_SYSCALLS
#include <lxlibcsysc.h>
#endif

#include <lxlist.h>

#define DevInt3() __asm__ __volatile__("int $3\n\t": : :"memory")

#define PAGE_SIZE    (4096)

#define O_LARGEFILE  (0100000)
#define O_DIRECTORY  (0200000)
//#define O_NOFOLLOW   (0400000)

extern HFILE lxapi_rt_hfile;
extern ULONG lxapi_sysstate;

extern void LXAPIENTRY LXAPI_exit(int rc);
extern LXAPISYSCRET LXAPIENTRY LXAPI_query_sysstate(unsigned long flags);
extern LXAPISYSCRET LXAPIENTRY LXAPI_init_runtime(void);
extern LXAPISYSCRET LXAPIENTRY LXAPI_term_runtime(void);
#ifdef LXISINIT
extern LXAPISYSCRET LXAPIENTRY LXAPI_init_process_started(void);
extern LXAPISYSCRET LXAPIENTRY LXAPI_init_process_finished(unsigned long flags);
#endif

#define main      LXAPI_main
#define exit      LXAPI_exit

#ifndef LX_NOWRAPPERS
#define open      LXAPI_open
#define _open     LXAPI_open
#define sopen     LXAPI_sopen
#define _sopen    LXAPI_sopen
#define creat     LXAPI_creat
#define _creat    LXAPI_creat
#define close     LXAPI_close
#define read      LXAPI_read
#define write     LXAPI_write
#define lseek     LXAPI_lseek
#define _lseek    LXAPI_lseek
#define mkdir     LXAPI_mkdir
#define rmdir     LXAPI_rmdir
#define link      LXAPI_link
#define unlink    LXAPI_unlink
#define remove    LXAPI_unlink
#define symlink   LXAPI_symlink
#define getcwd    LXAPI_getcwd
#define chdir     LXAPI_chdir
#define rename    LXAPI_rename
#define umask     LXAPI_umask
#define chroot    LXAPI_chroot
#define chmod     LXAPI_chmod
#define access    LXAPI_access
#define dup       LXAPI_dup
#define dup2      LXAPI_dup2



#endif // LX_NOWRAPPERS

#include <types.h>

extern int LXAPIENTRY LXAPI_open(__const__ char* pathname,int oflag,...);
extern int LXAPIENTRY LXAPI_sopen(__const__ char* pathname,int oflag,int sflag,...);
extern int LXAPIENTRY LXAPI_close(int handle);
extern int LXAPIENTRY LXAPI_creat(__const__ char* pathname,mode_t pmode);
extern ssize_t LXAPIENTRY LXAPI_read(int handle,void* buffer,size_t buflen);
extern int LXAPIENTRY LXAPI_write(int handle,__const__ void* buffer,size_t buflen);
extern off_t LXAPI_lseek(int handle,off_t offset,int origin);
extern int LXAPI_access(__const__ char* name,int mode);
extern int LXAPI_dup(int handle);
extern int LXAPI_dup2(int handle1,int handle2);

extern int LXAPI_link(__const__ char* oldname,__const__ char* newname);
extern int LXAPI_unlink(__const__ char* name);
extern int LXAPI_symlink(__const__ char* oldname,__const__ char* newname);

extern int LXAPI_mkdir(__const__ char* dirname,mode_t mode);
extern int LXAPI_rmdir(__const__ char* dirname);
extern char* LXAPI_getcwd(char* dirname,size_t buflen);
extern int LXAPI_chdir(__const__ char* dirname);
extern int LXAPI_rename(__const__ char* oldname,__const__ char* newname);
extern int LXAPI_chmod(__const__ char* name,mode_t pmode);

extern int LXAPI_umask(int pmode);

extern int stime(time_t* tptr);
extern int pause(void);
extern int nice(int increment);
extern int init_module(void* file,unsigned long len,__const__ char* args);
extern int delete_module(const char* name,unsigned int flags);

#ifdef LXFH_MAP
#define LXFH_TYPE_OS    (1)
#define LXFH_TYPE_LX    (2)
struct lx_filehandle
{
 struct list_head list;
 int type;
 int mapped_handle;
 int real_handle;
};

extern int LXAPIENTRY LXAPI_register_filehandle(int real_handle,int type);
extern int LXAPIENTRY LXAPI_register_real_filehandle(int real_handle
                                                     ,int mapped_handle
                                                     ,int type);
extern int LXAPIENTRY LXAPI_deregister_filehandle(int real_handle);
extern struct lx_filehandle* LXAPIENTRY
  LXAPI_get_filehandle_struct(int mapped_handle);
extern int LXAPIENTRY LXAPI_convert_omode_to_os(int omode);
extern int LXAPIENTRY LXAPI_convert_oflag_to_os(int oflag);

extern int LXAPIENTRY LXAPI_os2path_to_lx(const char* os2path,char* lxpath);
#endif //LXFH_MAP

#define streq(a,b) (strcmp((a),(b)) == 0)

#endif //LXLIBC_H_INCLUDED
