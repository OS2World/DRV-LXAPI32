/* $Id: ll_semaphore.c,v 1.2 2005/12/26 23:42:02 smilcke Exp $ */

/*
 * semaphore.c
 * Autor:               Stefan Milcke
 * Erstellt am:         22.04.2004
 * Letzte Aenderung am: 27.12.2005
 *
*/

#include <lxcommon.h>

#include <linux/config.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <asm/semaphore.h>

#include <lxapi.h>

/*
 * Semaphores are implemented using a two-way counter:
 * The "count" variable is decremented for each process
 * that tries to acquire the semaphore, while the "sleeping"
 * variable is a count of such acquires.
 *
 * Notably, the inline "up()" and "down()" functions can
 * efficiently test if they need to do any extra work (up
 * needs to do something only if count was negative before
 * the increment operation.
 *
 * "sleeping" and the contention routine ordering is protected
 * by the spinlock in the semaphore's waitqueue head.
 *
 * Note that these functions are only called when there is
 * contention on the lock, and as such all this is the
 * "non-critical" part of the whole semaphore business. The
 * critical part is the inline stuff in <asm/semaphore.h>
 * where we want to avoid any extra jumps and calls.
 */

/*
 * Logic:
 *  - only on a boundary condition do we need to care. When we go
 *    from a negative count to a non-negative, we wake people up.
 *  - when we go from a non-negative count to a negative do we
 *    (a) synchronize with the "sleeper" count and (b) make sure
 *    that we're on the wakeup list before we synchronize so that
 *    we cannot lose wakeup events.
 */

//------------------------------------ _up -------------------------------------
asmlinkage void _up(struct semaphore* sem)
{
 wake_up(&sem->wait);
}

//----------------------------------- _down ------------------------------------
asmlinkage void _down(struct semaphore * sem)
{
   struct task_struct *tsk = current;
   DECLARE_WAITQUEUE(wait, tsk);
   unsigned long flags;

   tsk->state = TASK_UNINTERRUPTIBLE;
   spin_lock_irqsave(&sem->wait.lock, flags);
   add_wait_queue_exclusive_locked(&sem->wait, &wait);

   sem->sleepers++;
   for (;;) {
      int sleepers = sem->sleepers;

      /*
       * Add "everybody else" into it. They aren't
       * playing, because we own the spinlock in
       * the wait_queue_head.
       */
      if (!atomic_add_negative(sleepers - 1, &sem->count)) {
         sem->sleepers = 0;
         break;
      }
      sem->sleepers = 1;   /* us - see -1 above */
      spin_unlock_irqrestore(&sem->wait.lock, flags);

      schedule();

      spin_lock_irqsave(&sem->wait.lock, flags);
      tsk->state = TASK_UNINTERRUPTIBLE;
   }
   remove_wait_queue_locked(&sem->wait, &wait);
   wake_up_locked(&sem->wait);
   spin_unlock_irqrestore(&sem->wait.lock, flags);
   tsk->state = TASK_RUNNING;
}

//---------------------------- _down_interruptible -----------------------------
asmlinkage int _down_interruptible(struct semaphore * sem)
{
   int retval = 0;
   struct task_struct *tsk = current;
   DECLARE_WAITQUEUE(wait, tsk);
   unsigned long flags;

   tsk->state = TASK_INTERRUPTIBLE;
   spin_lock_irqsave(&sem->wait.lock, flags);
   add_wait_queue_exclusive_locked(&sem->wait, &wait);

   sem->sleepers++;
   for (;;) {
      int sleepers = sem->sleepers;

      /*
       * With signals pending, this turns into
       * the trylock failure case - we won't be
       * sleeping, and we* can't get the lock as
       * it has contention. Just correct the count
       * and exit.
       */
      if (signal_pending(current)) {
         retval = -EINTR;
         sem->sleepers = 0;
         atomic_add(sleepers, &sem->count);
         break;
      }

      /*
       * Add "everybody else" into it. They aren't
       * playing, because we own the spinlock in
       * wait_queue_head. The "-1" is because we're
       * still hoping to get the semaphore.
       */
      if (!atomic_add_negative(sleepers - 1, &sem->count)) {
         sem->sleepers = 0;
         break;
      }
      sem->sleepers = 1;   /* us - see -1 above */
      spin_unlock_irqrestore(&sem->wait.lock, flags);

      schedule();

      spin_lock_irqsave(&sem->wait.lock, flags);
      tsk->state = TASK_INTERRUPTIBLE;
   }
   remove_wait_queue_locked(&sem->wait, &wait);
   wake_up_locked(&sem->wait);
   spin_unlock_irqrestore(&sem->wait.lock, flags);

   tsk->state = TASK_RUNNING;
   return retval;
}

/*
 * Trylock failed - make sure we correct for
 * having decremented the count.
 *
 * We could have done the trylock with a
 * single "cmpxchg" without failure cases,
 * but then it wouldn't work on a 386.
 */
//------------------------------- _down_trylock --------------------------------
asmlinkage int _down_trylock(struct semaphore * sem)
{
   int sleepers;
   unsigned long flags;

   spin_lock_irqsave(&sem->wait.lock, flags);
   sleepers = sem->sleepers + 1;
   sem->sleepers = 0;

   /*
    * Add "everybody else" and us into it. They aren't
    * playing, because we own the spinlock in the
    * wait_queue_head.
    */
   if (!atomic_add_negative(sleepers, &sem->count)) {
      wake_up_locked(&sem->wait);
   }

   spin_unlock_irqrestore(&sem->wait.lock, flags);
   return 1;
}


/*
 * The semaphore operations have a special calling sequence that
 * allow us to do a simpler in-line version of them. These routines
 * need to convert that sequence back into the C sequence when
 * there is contention on the semaphore.
 *
 * %ecx contains the semaphore pointer on entry. Save the C-clobbered
 * registers (%eax, %edx and %ecx) except %eax when used as a return
 * value..
 */

//------------------------------- __down_failed --------------------------------
asm(
 ".align 4\n"
#ifdef TARGET_OS2_GNU2
 ".globl _down_failed\n"
 "_down_failed:\n\t"
#else
 ".globl __down_failed\n"
 "__down_failed:\n\t"
#endif
#if defined(CONFIG_FRAME_POINTER)
 "pushl %ebp\n\t"
 "movl %esp,%ebp\n\t"
#endif
 "pushl %eax\n\t"
 "pushl %edx\n\t"
 "pushl %ecx\n\t"
#ifdef TARGET_OS2_GNU2
 "call _down\n\t"
#else
 "call __down\n\t"
#endif
 "popl %ecx\n\t"
 "popl %edx\n\t"
 "popl %eax\n\t"
#if defined(CONFIG_FRAME_POINTER)
 "movl %ebp,%esp\n\t"
 "popl %ebp\n\t"
#endif
 "ret"
);

//------------------------ __down_failed_interruptible -------------------------
asm(
".align 4\n"
#ifdef TARGET_OS2_GNU2
".globl _down_failed_interruptible\n"
"_down_failed_interruptible:\n\t"
#else
".globl __down_failed_interruptible\n"
"__down_failed_interruptible:\n\t"
#endif
#if defined(CONFIG_FRAME_POINTER)
 "pushl %ebp\n\t"
 "movl  %esp,%ebp\n\t"
#endif
 "pushl %edx\n\t"
 "pushl %ecx\n\t"
#ifdef TARGET_OS2_GNU2
 "call _down_interruptible\n\t"
#else
 "call __down_interruptible\n\t"
#endif
 "popl %ecx\n\t"
 "popl %edx\n\t"
#if defined(CONFIG_FRAME_POINTER)
 "movl %ebp,%esp\n\t"
 "popl %ebp\n\t"
#endif
 "ret"
);

//--------------------------- __down_failed_trylock ----------------------------
asm(
".align 4\n"
#ifdef TARGET_OS2_GNU2
".globl _down_failed_trylock\n"
"_down_failed_trylock:\n\t"
#else
".globl __down_failed_trylock\n"
"__down_failed_trylock:\n\t"
#endif
#if defined(CONFIG_FRAME_POINTER)
 "pushl %ebp\n\t"
 "movl  %esp,%ebp\n\t"
#endif
 "pushl %edx\n\t"
 "pushl %ecx\n\t"
#ifdef TARGET_OS2_GNU2
 "call _down_trylock\n\t"
#else
 "call __down_trylock\n\t"
#endif
 "popl %ecx\n\t"
 "popl %edx\n\t"
#if defined(CONFIG_FRAME_POINTER)
 "movl %ebp,%esp\n\t"
 "popl %ebp\n\t"
#endif
 "ret"
);

//-------------------------------- __up_wakeup ---------------------------------
asm(
".align 4\n"
#ifdef TARGET_OS2_GNU2
".globl _up_wakeup\n"
"_up_wakeup:\n\t"
#else
".globl __up_wakeup\n"
"__up_wakeup:\n\t"
#endif
 "pushl %eax\n\t"
 "pushl %edx\n\t"
 "pushl %ecx\n\t"
#ifdef TARGET_OS2_GNU2
 "call _up\n\t"
#else
 "call __up\n\t"
#endif
 "popl %ecx\n\t"
 "popl %edx\n\t"
 "popl %eax\n\t"
 "ret"
);

/*
 * rw spinlock fallbacks
 */
#if defined(CONFIG_SMP)
//---------------------------- __write_lock_failed -----------------------------
asm(
".align  4\n"
#ifdef TARGET_OS2_GNU2
".globl  _write_lock_failed\n"
"_write_lock_failed:\n\t"
#else
".globl  __write_lock_failed\n"
"__write_lock_failed:\n\t"
#endif
 LOCK "addl $" RW_LOCK_BIAS_STR ",(%eax)\n"
"1:   rep; nop\n\t"
 "cmpl   $" RW_LOCK_BIAS_STR ",(%eax)\n\t"
 "jne 1b\n\t"
 LOCK "subl $" RW_LOCK_BIAS_STR ",(%eax)\n\t"
#ifdef TARGET_OS2_GNU2
 "jnz _write_lock_failed\n\t"
#else
 "jnz __write_lock_failed\n\t"
#endif
 "ret"
);

//----------------------------- __read_lock_failed -----------------------------
asm(
".align  4\n"
#ifdef TARGET_OS2_GNU2
".globl  _read_lock_failed\n"
"_read_lock_failed:\n\t"
#else
".globl  __read_lock_failed\n"
"__read_lock_failed:\n\t"
#endif
 LOCK "incl (%eax)\n"
"1:   rep; nop\n\t"
 "cmpl   $1,(%eax)\n\t"
 "js  1b\n\t"
 LOCK "decl (%eax)\n\t"
#ifdef TARGET_OS2_GNU2
 "js  _read_lock_failed\n\t"
#else
 "js  __read_lock_failed\n\t"
#endif
 "ret"
);
#endif

