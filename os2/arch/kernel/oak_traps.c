/* $Id: oak_traps.c,v 1.2 2005/02/06 22:49:29 smilcke Exp $ */

/*
 * lak_traps.c
 * Autor:               Stefan Milcke
 * Erstellt am:         29.06.2004
 * Letzte Aenderung am: 29.01.2005
 *
*/

#include <lxcommon.h>

/*
 *  linux/arch/i386/traps.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  Pentium III FXSR, SSE support
 * Gareth Hughes <gareth@valinux.com>, May 2000
 */

/*
 * 'Traps.c' handles hardware traps and faults after we have saved some
 * state in 'asm.s'.
 */
#include <linux/config.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>
#include <linux/ptrace.h>
#include <linux/version.h>

#ifdef CONFIG_EISA
#include <linux/ioport.h>
#include <linux/eisa.h>
#endif

#ifdef CONFIG_MCA
#include <linux/mca.h>
#endif

#include <asm/processor.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/atomic.h>
// #include <asm/debugreg.h>
#include <asm/desc.h>
#include <asm/i387.h>
#include <asm/nmi.h>

#include <asm/smp.h>
#include <asm/pgalloc.h>
#include <asm/arch_hooks.h>

#include <linux/irq.h>
#include <linux/module.h>

struct desc_struct default_ldt[] = { { 0, 0 }, { 0, 0 }, { 0, 0 },
      { 0, 0 }, { 0, 0 } };

       /* Do we ignore FPU interrupts ? */
char ignore_fpu_irq = 0;

//------------------------------- default_do_nmi -------------------------------
static void default_do_nmi(struct pt_regs * regs)
{
}

//----------------------------- dummy_nmi_callback -----------------------------
static int dummy_nmi_callback(struct pt_regs * regs, int cpu)
{
   return 0;
}

static nmi_callback_t nmi_callback = dummy_nmi_callback;

//----------------------------------- do_nmi -----------------------------------
asmlinkage void do_nmi(struct pt_regs * regs, long error_code)
{
   int cpu;

   nmi_enter();

   cpu = smp_processor_id();
   ++nmi_count(cpu);

   if (!nmi_callback(regs, cpu))
      default_do_nmi(regs);

   nmi_exit();
}

//------------------------------ set_nmi_callback ------------------------------
void set_nmi_callback(nmi_callback_t callback)
{
   nmi_callback = callback;
}

//----------------------------- unset_nmi_callback -----------------------------
void unset_nmi_callback(void)
{
   nmi_callback = dummy_nmi_callback;
}

//------------------------------- show_registers -------------------------------
void show_registers(struct pt_regs *regs)
{
 printk("show_registers() called!\n");
}

//--------------------------------- show_trace ---------------------------------
void show_trace(struct task_struct *task, unsigned long * stack)
{
 unsigned long addr;
 if(!stack)
  stack=(unsigned long *)&stack;
 printk("Call Trace:\n");
 while(1)
 {
  struct thread_info* context=current_thread_info();
  while(!kstack_end(stack))
  {
   addr=*stack++;
   if(kernel_text_address(addr))
   {
    printk(" [<%08lx>] ",addr);
    print_symbol("%s\n",addr);
   }
  }
  printk(" ==============================\n");
  break;
 }
 printk("\n");
}

//--------------------------------- show_stack ---------------------------------
void show_stack(struct task_struct *task, unsigned long *esp)
{
 printk("show_stack() called!\n");
}

//--------------------------------- math_error ---------------------------------
void math_error(void* eip)
{
}

/*
 * This needs to use 'idt_table' rather than 'idt', and
 * thus use the _nonmapped_ version of the IDT, as the
 * Pentium F0 0F bugfix can have resulted in the mapped
 * IDT being write-protected.
 */
void set_intr_gate(unsigned int n, void *addr)
{
}

static void __init set_trap_gate(unsigned int n, void *addr)
{
}

static void __init set_system_gate(unsigned int n, void *addr)
{
}

static void __init set_call_gate(void *a, void *addr)
{
}

static void __init set_task_gate(unsigned int n, unsigned int gdt_entry)
{
}

//--------------------------------- trap_init ----------------------------------
void __init trap_init(void)
{
}
