/* $Id: of_binfmt_aout.c,v 1.4 2006/04/06 21:17:43 smilcke Exp $ */

/*
 * lf_binfmt_aout.c
 * Autor:               Stefan Milcke
 * Erstellt am:         26.06.2004
 * Letzte Aenderung am: 22.03.2005
 *
*/

#define LXMALLOC_DECLARATIONS
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
#include <linux/stat.h>
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

/*
 *  linux/fs/binfmt_aout.c
 *
 *  Copyright (C) 1991, 1992, 1996  Linus Torvalds
 */


struct lx_aout_binary // Move me to lxsegments.h
{
 struct list_head list;
 struct lx_segment* cs;
 struct lx_segment* ds;
 struct lx_segment* bss;
 unsigned long sym_start;
 unsigned long sym_end;
};

static int load_aout_binary(struct linux_binprm *, struct pt_regs * regs);
static int load_aout_library(struct file*);
static int aout_core_dump(long signr, struct pt_regs * regs, struct file *file);

extern void dump_thread(struct pt_regs *, struct user *);

static struct linux_binfmt aout_format = {
   .module     = THIS_MODULE,
   .load_binary   = load_aout_binary,
   .load_shlib = load_aout_library,
   .core_dump  = aout_core_dump,
   .min_coredump  = PAGE_SIZE
};

static void set_brk(unsigned long start, unsigned long end)
{
   start = PAGE_ALIGN(start);
   end = PAGE_ALIGN(end);
   if (end <= start)
      return;
   do_brk(start, end - start);
}

/*
 * Routine writes a core dump image in the current directory.
 * Currently only a stub-function.
 *
 * Note that setuid/setgid files won't make a core-dump if the uid/gid
 * changed due to the set[u|g]id. It's enforced by the "current->mm->dumpable"
 * field, which also makes sure the core-dumps won't be recursive if the
 * dumping of the process results in another error..
 */

static int aout_core_dump(long signr, struct pt_regs * regs, struct file *file)
{
 return 1;
}

/*
 * create_aout_tables() parses the env- and arg-strings in new user
 * memory and creates the pointer tables from them, and puts their
 * addresses on the "stack", returning the new stack pointer value.
 */
static unsigned long * create_aout_tables(char * p, struct linux_binprm * bprm)
{
 return 0;
}

#ifdef TARGET_OS2
static struct nlist* get_symbol(int symbol_num,struct nlist* symbuf,unsigned long syms)
{
 if(symbol_num*sizeof(struct nlist)>syms)
  return 0;
 else
  return &(symbuf[symbol_num]);
}

//------------------------------ get_nlist_struct ------------------------------
static struct nlist* get_nlist_struct(void* buffer,int symbol_ord)
{
 struct exec* ex=(struct exec*)buffer;
 struct nlist* nl=(struct nlist*)(((unsigned long)buffer)
                                  +sizeof(struct exec)
                                  +ex->a_text+ex->a_data+ex->a_bss
                                  +ex->a_trsize+ex->a_drsize);
 if(symbol_ord*sizeof(struct nlist)>ex->a_syms)
  return 0;
 else
  return &(nl[symbol_ord]);

}

//---------------------------- recalc_nlist_entries ----------------------------
static int recalc_nlist_entries(void* buffer)
{
 struct exec* ex=(struct exec*)buffer;
 struct nlist* nl=(struct nlist*)(((unsigned long)buffer)
                                  +sizeof(struct exec)
                                  +ex->a_text+ex->a_data+ex->a_bss
                                  +ex->a_trsize+ex->a_drsize);
 int syms=ex->a_syms;
 char* txt_buffer=((char*)nl)+ex->a_syms;
 while(syms>0)
 {
  nl->n_un.n_name=txt_buffer+nl->n_un.n_strx;
  nl++;
  syms-=sizeof(struct nlist);
 }
 return 0;
}

//---------------------------- resolve_relocations -----------------------------
static int resolve_relocations(void* buffer,struct lx_aout_binary* bin
                               ,struct relocation_info* reloc
                               ,int rsz,int isDataReloc)
{
 int retval=0;
 while(rsz>0)
 {
  if(reloc->r_extern)
  {
  }
  else
  {
   unsigned long off=reloc->r_address;
  }
  reloc++;
  rsz-=sizeof(struct relocation_info);
 }
 return retval;
}

//---------------------------- resolve_aout_binary -----------------------------
static int resolve_aout_binary(void* buffer,struct lx_aout_binary* bin)
{
 int retval;
 struct exec* ex=(struct exec*)buffer;
 struct relocation_info* reloc;
 retval=recalc_nlist_entries(buffer);
 DevInt3();
 if(ex->a_trsize)
 {
  reloc=(struct relocation_info*)(((unsigned long)buffer)
                                  +ex->a_text+ex->a_data
                                  +sizeof(struct exec));
  retval=resolve_relocations(buffer,bin,reloc,ex->a_trsize,0);
 }
 if(ex->a_drsize)
 {
  reloc=(struct relocation_info*)(((unsigned long)buffer)
                                  +ex->a_text+ex->a_data+ex->a_drsize
                                  +sizeof(struct exec));
  retval=resolve_relocations(buffer,bin,reloc,ex->a_drsize,1);
 }
 retval=-ENOEXEC;
 return retval;
}

/*
 * These are the functions used to load a.out style executables and shared
 * libraries.  There is no binary dependent code anywhere else.
 */
static int load_aout_binary(struct linux_binprm* bprm,struct pt_regs* regs)
{
 int retval=-ENOEXEC;
 struct exec ex;
 int fsize;
 void* buffer;
 struct lx_aout_binary* bin;

 ex = *((struct exec *) bprm->buf);     /* exec-header */
 fsize=i_size_read(bprm->file->f_dentry->d_inode);
 if ((N_MAGIC(ex) != ZMAGIC && N_MAGIC(ex) != OMAGIC &&
      N_MAGIC(ex) != QMAGIC && N_MAGIC(ex) != NMAGIC) ||
     fsize < ex.a_text+ex.a_data+N_SYMSIZE(ex)+N_TXTOFF(ex))
 {
  return -ENOEXEC;
 }
 buffer=LX_malloc(fsize);
 if(!buffer)
  return -ENOMEM;
 bin=kmalloc(sizeof(struct lx_aout_binary),GFP_KERNEL);
 if(!bin)
  goto out_no_bin;
 bin->cs= LX_alloc_segment(GDTSEL_R0CODE32,PAGE_ALIGN(ex.a_text),0,0);
 bin->ds= LX_alloc_segment(GDTSEL_R0DATA  ,PAGE_ALIGN(ex.a_data),0,0);
 bin->bss=LX_alloc_segment(GDTSEL_R0DATA  ,PAGE_ALIGN(ex.a_bss),0,0);
 if(!bin->cs || !bin->ds || !bin->bss)
  goto out_no_segments;
 if(buffer && bin)
 {
  loff_t pos=0;
  bprm->file->f_op->read(bprm->file,(char*)buffer,fsize,&pos);
 }
 retval=resolve_aout_binary(buffer,bin);
 if(!retval)
 {
  // Free temporary buffer
  LX_free(buffer);
  return 0;
 }
 // Fall through
out_no_segments:
 LX_free_segment(bin->cs);
 LX_free_segment(bin->ds);
 LX_free_segment(bin->bss);
 kfree(bin);
out_no_bin:
 LX_free(buffer);
 return retval;
}

// **** OLD ROUTINES ****
#if 0
static int resolve_aout_binary_old(struct linux_binprm* bprm,loff_t pos
                               ,struct exec* ex
                               ,struct lx_segment* cs
                               ,struct lx_segment* ds
                               ,struct lx_segment* bss
                               ,unsigned long *entry_point)
{
 char* code_reloc_buffer=0;
 char* data_reloc_buffer=0;
 char* sym_buffer=0;
 char* txt_buffer=0;
 struct relocation_info* reloc;
 struct nlist* nlist;
 int trsize=ex->a_trsize;
 int drsize=ex->a_drsize;
 int syms=ex->a_syms;
 int txts=0;
 int retval=-ENOMEM;
 if(trsize)
 {
  code_reloc_buffer=kmalloc(trsize,GFP_KERNEL);
  bprm->file->f_op->read(bprm->file,code_reloc_buffer,trsize,&pos);
 }
 if(drsize)
 {
  data_reloc_buffer=kmalloc(drsize,GFP_KERNEL);
  bprm->file->f_op->read(bprm->file,data_reloc_buffer,drsize,&pos);
 }
 if(syms)
 {
  loff_t tpos=pos;
  sym_buffer=kmalloc(syms,GFP_KERNEL);
  bprm->file->f_op->read(bprm->file,sym_buffer,syms,&pos);
  bprm->file->f_op->read(bprm->file,(char*)&txts,sizeof(unsigned long),&pos);
  if(txts)
  {
   txt_buffer=kmalloc(txts,GFP_KERNEL);
   bprm->file->f_op->read(bprm->file,txt_buffer,txts,&pos);
   nlist=(struct nlist*)sym_buffer;
   while(syms>0)
   {
    nlist->n_un.n_name=txt_buffer+nlist->n_un.n_strx-sizeof(unsigned long);
    nlist++;
    syms-=sizeof(struct nlist);
   }
  }
 }
 DevInt3();
 reloc=(struct relocation_info*)code_reloc_buffer;
 trsize=ex->a_trsize;
 while(trsize>0)
 {
  if(reloc->r_extern)
  {
  }
  else
  {
   unsigned long val;
   unsigned long virt;
   unsigned long real;
   unsigned long off;
   DevInt3();
   off=reloc->r_address;
   if(off>ex->a_text)
   {
    virt=(unsigned long)ds->pVirtual;
    real=(unsigned long)ds->pMapped;
    off-=ex->a_text;
   }
   else
   {
    virt=(unsigned long)cs->pVirtual;
    real=(unsigned long)cs->pMapped;
   }
   val=*((unsigned long*)(virt+off));
  }
  reloc++;
  trsize-=sizeof(struct relocation_info);
 }
 DevInt3();
 reloc=(struct relocation_info*)data_reloc_buffer;
 drsize=ex->a_drsize;
 while(drsize>0)
 {
  reloc++;
  drsize-=sizeof(struct relocation_info);
 }
 syms=ex->a_syms;
 nlist=(struct nlist*)sym_buffer;
 while(syms>0)
 {
  nlist++;
  syms-=sizeof(struct nlist);
 }
 kfree(code_reloc_buffer);
 kfree(data_reloc_buffer);
 kfree(sym_buffer);
 kfree(txt_buffer);
 return retval;
}

static int load_aout_binary_old(struct linux_binprm * bprm, struct pt_regs * regs)
{
   struct exec ex;
   unsigned long error;
   unsigned long fd_offset;
   unsigned long rlim;
   int retval;
   unsigned long entry_point=0;
   struct lx_segment* lx_cs;
   struct lx_segment* lx_ds;
   struct lx_segment* lx_bss;
   loff_t pos=32;

   ex = *((struct exec *) bprm->buf);     /* exec-header */
   if ((N_MAGIC(ex) != ZMAGIC && N_MAGIC(ex) != OMAGIC &&
        N_MAGIC(ex) != QMAGIC && N_MAGIC(ex) != NMAGIC) ||
       i_size_read(bprm->file->f_dentry->d_inode) < ex.a_text+ex.a_data+N_SYMSIZE(ex)+N_TXTOFF(ex)) {
      return -ENOEXEC;
   }
   fd_offset = N_TXTOFF(ex);

   /* Check initial limits. This avoids letting people circumvent
    * size limits imposed on them by creating programs with large
    * arrays in the data or bss.
    */
   rlim = current->rlim[RLIMIT_DATA].rlim_cur;
   if (rlim >= RLIM_INFINITY)
      rlim = ~0;
   if (ex.a_data + ex.a_bss > rlim)
      return -ENOMEM;

   lx_cs=LX_alloc_segment(GDTSEL_R0CODE32,PAGE_ALIGN(ex.a_text));
   lx_ds=LX_alloc_segment(GDTSEL_R0DATA,PAGE_ALIGN(ex.a_data));
   lx_bss=LX_alloc_segment(GDTSEL_R0DATA,PAGE_ALIGN(ex.a_bss));
   bprm->file->f_op->read(bprm->file,(char*)lx_cs->pVirtual,ex.a_text,&pos);
   bprm->file->f_op->read(bprm->file,(char*)lx_ds->pVirtual,ex.a_data,&pos);
   bprm->file->f_op->read(bprm->file,(char*)lx_bss->pVirtual,ex.a_bss,&pos);
   retval=resolve_aout_binary_old(bprm,pos,&ex,lx_cs,lx_ds,lx_bss,&entry_point);
   if(retval)
   {
    LX_free_segment(lx_cs);
    LX_free_segment(lx_ds);
    LX_free_segment(lx_bss);
   }
   return retval;
}
#endif // 0


static int load_aout_library(struct file *file)
{
 return -ENOEXEC;
}

#endif // TARGET_OS2

static int __init init_aout_binfmt(void)
{
   return register_binfmt(&aout_format);
}

static void __exit exit_aout_binfmt(void)
{
   unregister_binfmt(&aout_format);
}

module_init(init_aout_binfmt);
module_exit(exit_aout_binfmt);
MODULE_LICENSE("GPL");
