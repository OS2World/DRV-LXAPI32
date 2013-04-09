/* $Id: oof_binfmt_lxexe.c,v 1.7 2005/04/18 22:28:45 smilcke Exp $ */

/*
 * oof_binfmt_lxexe.c
 * Autor:               Stefan Milcke
 * Erstellt am:         28.02.2005
 * Letzte Aenderung am: 11.04.2005
 *
*/

#include <lxcommon.h>
#include <linux/module.h>

#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/a.out.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/file.h>

#include <linux/fcntl.h>
#include <linux/ptrace.h>
#include <linux/user.h>
#include <linux/slab.h>
#include <linux/binfmts.h>
#include <linux/personality.h>
#include <linux/init.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/pgalloc.h>
#include <asm/cacheflush.h>
#include <linux/security.h>

#include <lxdaemon.h>

static int load_lxexe_binary(struct linux_binprm* bprm,struct pt_regs* regs);
static int lxexe_core_dump(long signr,struct pt_regs* regs,struct file* file);

static struct linux_binfmt lxexe_format=
{
 .module=THIS_MODULE,
 .load_binary=load_lxexe_binary,
 .load_shlib=0,
 .core_dump=lxexe_core_dump,
 .min_coredump=PAGE_SIZE
};

static void set_brk(unsigned long start, unsigned long end)
{
   start = PAGE_ALIGN(start);
   end = PAGE_ALIGN(end);
   if (end <= start)
      return;
   do_brk(start, end - start);
}

//------------------------------ lxexe_core_dump -------------------------------
static int lxexe_core_dump(long signr,struct pt_regs* regs,struct file* file)
{
 return -ENOMEM;
}

#define MAX_ARG_LEN  (1024)
extern void LX_UtilStackWaitForCaptureAndExit(unsigned short pid,unsigned short tid
                                              ,unsigned long no_exit_thread);
//----------------------------- load_lxexe_binary ------------------------------
static int load_lxexe_binary(struct linux_binprm* bprm,struct pt_regs* regs)
{
 int retval;
 char* fn=0;
 char* failname=0;
 char* args=0;
 char* pa;
 int arglen;
 RESULTCODES resultcodes;
 if(bprm->buf[0]!='M' || bprm->buf[1]!='Z')
  return -ENOEXEC;
 fn=(char*)kmalloc(LX_MAXPATH,GFP_KERNEL);
 failname=(char*)kmalloc(LX_MAXPATH,GFP_KERNEL);
 args=(char*)kmalloc(MAX_ARG_LEN,GFP_KERNEL);
 if(!fn || !failname || !args)
 {
  retval=-ENOMEM;
  goto err_out;
 }
 retval=LX_os2path_from_file(bprm->file,fn,LX_MAXPATH);
 if(retval)
  goto err_out;
 memset(args,0,MAX_ARG_LEN);
 memset(failname,0,LX_MAXPATH);
 strcpy(args,fn);
 arglen=strlen(args)+1;
 pa=(char*)(args+arglen);
 strcpy(pa,bprm->filename);
 arglen+=strlen(pa)+1;
 retval=LX_DOSEXECPGM(failname,LX_MAXPATH,EXEC_ASYNC
                      ,args,arglen
                      ,NULL,0
                      ,&resultcodes,fn);
 if(!retval)
 {
  int i;
  retval=flush_old_exec(bprm);
  if(retval)
   return retval;
  set_binfmt(&lxexe_format);
  set_brk(current->mm->start_brk,current->mm->brk);
  for(i=0;i<MAX_ARG_PAGES;i++)
  {
   if(bprm->page[i])
    __free_page(bprm->page[i]);
   bprm->page[i]=0;
  }
  security_bprm_free(bprm);
  if(bprm->mm)
  {
   mmdrop(bprm->mm);
   bprm->mm=0;
  }
  if(bprm->file)
  {
   allow_write_access(bprm->file);
   fput(bprm->file);
   bprm->file=0;
  }
  kfree(fn);
  kfree(failname);
  kfree(args);
  {
   struct thread_info* ti=current_thread_info();
   int is_utl_current=LX_is_utl_current(ti);
   if(is_utl_current)
    LX_unset_ti_for_pidtid(ti,lx_current_pid,lx_current_tid);
   LX_set_ti_for_pidtid(ti,resultcodes.codeTerminate,1);
   LX_UtilStackWaitForCaptureAndExit(resultcodes.codeTerminate,1,is_utl_current);
  }
 }
 else
 {
  printk("DosExecPgm(%s) failed\n",fn);
  printk("Result code   : %d\n",retval);
  printk("Failing module: %s\n",failname);
 }
err_out:
 kfree(fn);
 kfree(failname);
 kfree(args);
 return retval;
}

//----------------------------- init_lxexe_binfmt ------------------------------
static int __init init_lxexe_binfmt(void)
{
 return register_binfmt(&lxexe_format);
}

//----------------------------- exit_lxexe_binfmt ------------------------------
static void __exit exit_lxexe_binfmt(void)
{
 unregister_binfmt(&lxexe_format);
}

module_init(init_lxexe_binfmt);
module_exit(exit_lxexe_binfmt);
MODULE_LICENSE("GPL");
