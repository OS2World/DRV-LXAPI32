/* $Id: lxcommon.h,v 1.23 2005/12/26 23:42:00 smilcke Exp $ */

/*
 * lxcommon.h
 * Autor:               Stefan Milcke
 * Erstellt am:         04.03.2004
 * Letzte Aenderung am: 24.12.2005
 *
*/

#ifndef LXCOMMON_H_INCLUDED
#define LXCOMMON_H_INCLUDED

#if __GNUC__ < 3
#define TARGET_OS2_GNU2
#endif

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <asm/atomic.h>

#define INCL_NOPMAPI
#define INCL_DOSERRORS           // for ERROR_INVALID_FUNCTION
#define INCL_DOSPROCESS
#include <os2.h>

#ifndef LX_MODULE_NAME
#define LX_MODULE_NAME kernel
#endif

#include <devhlp32.h>
#include <devtype.h>
#include <defconfig.h>

#include <linux/wait.h>

#ifndef SIS_LEN // infoseg.h has no multiple include lock
#include <infoseg.h>
#endif

#include <lxmemory.h>
#include <lxsegments.h>
#include <lxstack.h>
#include <lxfs.h>
#include <lxsysstate.h>

#ifdef LXDEBUG
extern volatile unsigned long lx_call_depth;
#endif

#define LX_MAXPATH   (260)

extern struct InfoSegGDT* lx_InfoSegGDTPtr;
extern struct InfoSegLDT* lx_InfoSegLDTPtr;
extern unsigned short* P_INIT_COUNT;
extern unsigned int lx_num_cpus;
extern int lx_is_smp;

extern atomic_t lx_exit_kernel;
extern atomic_t lx_kernel_running;
extern atomic_t lx_current_available;
extern atomic_t lx_in_ISR;
extern volatile unsigned long lxa_shutdown_level;
extern wait_queue_head_t lx_shutdown_wq;

extern struct task_struct* lx_idle_task;
extern struct task_struct* lx_init_task;

extern void LX_enter_current(void);
extern void LX_leave_current(void);
extern void LX_enter_irq_current(void);
extern void LX_leave_irq_current(void);
extern void LX_enter_utl_current(void);
extern void LX_leave_utl_current(int flags);
#define LXLEAVE_UTL_DOEXIT 1
#define LXLEAVE_UTL_FREEME 2

extern void LX_set_current_task(struct task_struct* tsk);
extern int LX_is_utl_current(struct thread_info* ti);

#define MAX_GDTSELECTORS   64 // Must match MAX_GDTSELECTORS in startup.asm

extern void LX_show_trace(void);

#define NOT_IMPLEMENTED() { printk("In Function %s{%s}: Not Implemented!\n",__FUNCTION__,__FILE__); \
                            LX_show_trace(); \
                            DevInt3(); }

extern unsigned short fastcall _lx_current_pid(void);
extern unsigned short fastcall _lx_current_tid(void);

#define lx_current_pid (lx_is_smp ? _lx_current_pid() : (lx_InfoSegLDTPtr->LIS_CurProcID))
#define lx_current_tid (lx_is_smp ? _lx_current_tid() : (lx_InfoSegLDTPtr->LIS_CurThrdID))

#define IRQ_FLAG_IMMEDIATE_EOI   1
#define IRQ_FLAG_CLI_BEFORE_EOI  2
#define IRQ_FLAG_STI_AFTER_EOI   4

#define KERNEL_VERSION(a,b,c) (((a) << 16) | ((b) << 8) | (c))

/* Thread/Task management */
extern unsigned long LX_WaitOn(unsigned long id
                               ,unsigned long timeout
                               ,unsigned long flags);
extern unsigned long LX_RunOn(unsigned long id);
#define LX_STARTTASK_WAIT      1
#define LX_STARTTASK_STARTED   2
#define LX_STARTTASK_NOBLOCK   4
extern fastcall unsigned long LXA_CreateThread(unsigned long fnptr,unsigned long* ptid);
extern struct task_struct* LX_switch_to(struct task_struct* prev,struct task_struct *next);
extern unsigned long LX_StartNewTask(unsigned long esp,unsigned long eip
                                     ,unsigned long prev
                                     ,unsigned long flags);
extern void LX_set_task_regular_exit(struct task_struct* tsk);

/* misc */
extern char lx_log_file_name[];
extern char lx_log_path[];
extern char lx_root_path[];

extern void LX_set_continue_startup(void);
extern unsigned long LX_write_log(char* logData);

/* screen IO at startup */
#define LX_SCRFLG_SETCHAR        (1)
#define LX_SCRFLG_SETATTR        (2)
#define LX_SCRFLG_NOUPDCSR       (4)

#define LX_SCRATT_BLACK          (0)
#define LX_SCRATT_DARK_BLUE      (1)
#define LX_SCRATT_DARK_GREEN     (2)
#define LX_SCRATT_DARK_CYAN      (3)
#define LX_SCRATT_DARK_RED       (4)
#define LX_SCRATT_VIOLET         (5)
#define LX_SCRATT_BROWN          (6)
#define LX_SCRATT_GREY           (7)
#define LX_SCRATT_DARK_GREY      (8)
#define LX_SCRATT_LIGHT_BLUE     (9)
#define LX_SCRATT_LIGHT_GREEN    (10)
#define LX_SCRATT_LIGHT_CYAN     (11)
#define LX_SCRATT_LIGHT_RED      (12)
#define LX_SCRATT_LIGHT_VIOLET   (13)
#define LX_SCRATT_YELLOW         (14)
#define LX_SCRATT_WHITE          (15)

#define LX_SCRATT_FLASH          (128)
extern unsigned long LX_open_screen(void);
extern unsigned long LX_close_screen(void);
extern unsigned long LX_scr_scroll_up(void);
extern unsigned long LX_scr_put_char_at(int* row,int* col,char c,char attr,int flags);
extern unsigned long LX_scr_put_string_at(int* row,int* col,char* msg);
extern unsigned long LX_scr_put_string(char* msg);
extern unsigned long LX_clear_screen(void);

static inline long LX_GET1616PTRCONTENTSL(ptr)
{
 long rc;
 __asm__ __volatile__
 (
  "pushw %%es\n\t"
  "movw  %%ax,%%es\n\t"
  "movl  %%es:(%%ebx),%%eax\n\t"
  "popw  %%es\n\t"
  :"=a"((long)rc)
  :"a"((short)SELECTOROF(ptr)), "b"((long)OFFSETOF(ptr))
  :"cc"
 );
 return rc;
}

static inline short LX_GET1616PTRCONTENTSS(ptr)
{
 short rc;
 __asm__ __volatile__
 (
  "pushw %%es\n\t"
  "movw  %%ax,%%es\n\t"
  "movw  %%es:(%%ebx),%%ax\n\t"
  "popw  %%es\n\t"
  :"=a"((short)rc)
  :"a"((short)SELECTOROF(ptr)), "b"((long)OFFSETOF(ptr))
  :"cc"
 );
 return rc;
}

static inline char LX_GET1616PTRCONTENTSB(ptr)
{
 char rc;
 __asm__ __volatile__
 (
  "pushw %%es\n\t"
  "movw  %%ax,%%es\n\t"
  "movb  %%es:(%%ebx),%%al\n\t"
  "popw  %%es\n\t"
  :"=a"((char)rc)
  :"a"((short)SELECTOROF(ptr)), "b"((long)OFFSETOF(ptr))
  :"cc"
 );
 return rc;
}

#endif //LXCOMMON_H_INCLUDED
