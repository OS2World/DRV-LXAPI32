/* $Id: lxszopt.c,v 1.4 2006/01/05 23:48:18 smilcke Exp $ */

/*
 * lxszopt.c
 * Autor:               Stefan Milcke
 * Erstellt am:         09.05.2005
 * Letzte Aenderung am: 29.12.2005
 *
*/

#include <lxcommon.h>

#ifdef CONFIG_LX_OPTIMIZE_SIZE

#include <linux/module.h>
#include <linux/smp_lock.h>
#include <asm/semaphore.h>

//---------------------------- current_thread_info -----------------------------
struct thread_info* current_thread_info(void)
{
 struct thread_info *ti;
 __asm__("andl %%esp,%0; ":"=r" (ti) : "0" (~(THREAD_SIZE - 1)));
 return ti;
}
EXPORT_SYMBOL(current_thread_info);

//-------------------------------- get_current ---------------------------------
struct task_struct* get_current(void)
{
 return current_thread_info()->task;
}
EXPORT_SYMBOL(get_current);

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)

//---------------------------- release_kernel_lock -----------------------------
void release_kernel_lock(struct task_struct *task)
{
   if (unlikely(task->lock_depth >= 0))
      put_kernel_lock();
}
EXPORT_SYMBOL(release_kernel_lock);

//--------------------------- reacquire_kernel_lock ----------------------------
void reacquire_kernel_lock(struct task_struct *task)
{
   if (unlikely(task->lock_depth >= 0))
      get_kernel_lock();
}
EXPORT_SYMBOL(reacquire_kernel_lock);

//-------------------------------- lock_kernel ---------------------------------
void lock_kernel(void)
{
   int depth = current->lock_depth+1;
   if (likely(!depth))
      get_kernel_lock();
   current->lock_depth = depth;
}
EXPORT_SYMBOL(lock_kernel);

//------------------------------- unlock_kernel --------------------------------
void unlock_kernel(void)
{
   BUG_ON(current->lock_depth < 0);
   if (likely(--current->lock_depth < 0))
      put_kernel_lock();
}
EXPORT_SYMBOL(unlock_kernel);
#endif

//------------------------------------ down ------------------------------------
void down(struct semaphore* sem)
{
#ifdef WAITQUEUE_DEBUG
   CHECK_MAGIC(sem->__magic);
#endif
   might_sleep();
   __asm__ __volatile__(
   "# atomic down operation\n\t"
   LOCK "decl %0\n\t"        /* --sem->count */
   "jns 1f\n\t"
#ifdef TARGET_OS2_GNU2
   "call _down_failed\n\t"
#else
   "call __down_failed\n\t"
#endif
   "1:\n"
   :"=m" (sem->count)
   :"c" (sem)
   :"memory");
}
EXPORT_SYMBOL(down);

//----------------------------- down_interruptible -----------------------------
int down_interruptible(struct semaphore* sem)
{
 int result;
#ifdef WAITQUEUE_DEBUG
 CHECK_MAGIC(sem->__magic);
#endif
 might_sleep();
 __asm__ __volatile__(
 "# atomic interruptible down operation\n\t"
 LOCK "decl %1\n\t"  /* --sem->count */
 "jns 1f\n\t"
#ifdef TARGET_OS2_GNU2
 "call _down_failed_interruptible\n\t"
#else
 "call __down_failed_interruptible\n\t"
#endif
 "jmp 2f\n\t"
 "1:\txorl %0,%0\n\t"
 "2:\n"
 :"=a" (result), "=m" (sem->count)
 :"c" (sem)
 :"memory");
 return result;
}
EXPORT_SYMBOL(down_interruptible);

//-------------------------------- down_trylock --------------------------------
int down_trylock(struct semaphore* sem)
{
 int result;
#ifdef WAITQUEUE_DEBUG
 CHECK_MAGIC(sem->__magic);
#endif
 __asm__ __volatile__(
 "# atomic interruptible down operation\n\t"
 LOCK "decl %1\n\t"  /* --sem->count */
 "jns 1f\n\t"
#ifdef TARGET_OS2_GNU2
 "call _down_failed_trylock\n\t"
#else
 "call __down_failed_trylock\n\t"
#endif
 "jmp 2f\n\t"
 "1:\txorl %0,%0\n\t"
 "2:\n"
 :"=a" (result), "=m" (sem->count)
 :"c" (sem)
 :"memory");
 return result;
}
EXPORT_SYMBOL(down_trylock);

//------------------------------------- up -------------------------------------
void up(struct semaphore* sem)
{
#ifdef WAITQUEUE_DEBUG
 CHECK_MAGIC(sem->__magic);
#endif
 __asm__ __volatile__(
 "# atomic up operation\n\t"
 LOCK "incl %0\n\t"
 "jg 1f\n\t"
#ifdef TARGET_OS2_GNU2
 "call _up_wakeup\n\t"
#else
 "call __up_wakeup\n\t"
#endif
 "1:\n\t"
 :"=m" (sem->count)
 :"c" (sem)
 :"memory");
}
EXPORT_SYMBOL(up);

#if !defined(CONFIG_X86_OOSTORE) && !defined(CONFIG_X86_PPRO_FENCE)
//------------------------------ _raw_spin_unlock ------------------------------
void _raw_spin_unlock(spinlock_t *lock)
{
#ifdef CONFIG_DEBUG_SPINLOCK
   BUG_ON(lock->magic != SPINLOCK_MAGIC);
   BUG_ON(!spin_is_locked(lock));
#endif
   __asm__ __volatile__(
      spin_unlock_string
   );
}
EXPORT_SYMBOL(_raw_spin_unlock);

#else

//------------------------------ _raw_spin_unlock ------------------------------
void _raw_spin_unlock(spinlock_t *lock)
{
   char oldval = 1;
#ifdef CONFIG_DEBUG_SPINLOCK
   BUG_ON(lock->magic != SPINLOCK_MAGIC);
   BUG_ON(!spin_is_locked(lock));
#endif
   __asm__ __volatile__(
      spin_unlock_string
   );
}
EXPORT_SYMBOL(_raw_spin_unlock);

#endif

//----------------------------- _raw_spin_trylock ------------------------------
int _raw_spin_trylock(spinlock_t *lock)
{
   char oldval;
   __asm__ __volatile__(
      "xchgb %b0,%1"
      :"=q" (oldval), "=m" (lock->lock)
      :"0" (0) : "memory");
   return oldval > 0;
}
EXPORT_SYMBOL(_raw_spin_trylock);

//------------------------------- _raw_spin_lock -------------------------------
void _raw_spin_lock(spinlock_t *lock)
{
#ifdef CONFIG_DEBUG_SPINLOCK
   __label__ here;
here:
   if (unlikely(lock->magic != SPINLOCK_MAGIC)) {
      printk("eip: %p\n", &&here);
      BUG();
   }
#endif
   __asm__ __volatile__(
      spin_lock_string
      :"=m" (lock->lock) : : "memory");
}
EXPORT_SYMBOL(_raw_spin_lock);

//------------------------------- _raw_read_lock -------------------------------
void _raw_read_lock(rwlock_t *rw)
{
#ifdef CONFIG_DEBUG_SPINLOCK
   BUG_ON(rw->magic != RWLOCK_MAGIC);
#endif
#ifdef TARGET_OS2_GNU2
   __build_read_lock(rw, "_read_lock_failed");
#else
   __build_read_lock(rw, "__read_lock_failed");
#endif
}
EXPORT_SYMBOL(_raw_read_lock);

//------------------------------ _raw_write_lock -------------------------------
void _raw_write_lock(rwlock_t *rw)
{
#ifdef CONFIG_DEBUG_SPINLOCK
   BUG_ON(rw->magic != RWLOCK_MAGIC);
#endif
#ifdef TARGET_OS2_GNU2
   __build_write_lock(rw, "_write_lock_failed");
#else
   __build_write_lock(rw, "__write_lock_failed");
#endif
}
EXPORT_SYMBOL(_raw_write_lock);

//----------------------------- _raw_write_trylock -----------------------------
int _raw_write_trylock(rwlock_t *lock)
{
   atomic_t *count = (atomic_t *)lock;
   if (atomic_sub_and_test(RW_LOCK_BIAS, count))
      return 1;
   atomic_add(RW_LOCK_BIAS, count);
   return 0;
}
EXPORT_SYMBOL(_raw_write_trylock);

//--------------------------------- atomic_add ---------------------------------
void atomic_add(int i, atomic_t *v)
{
   __asm__ __volatile__(
      LOCK "addl %1,%0"
      :"=m" (v->counter)
      :"ir" (i), "m" (v->counter));
}
EXPORT_SYMBOL(atomic_add);

//--------------------------------- atomic_sub ---------------------------------
void atomic_sub(int i, atomic_t *v)
{
   __asm__ __volatile__(
      LOCK "subl %1,%0"
      :"=m" (v->counter)
      :"ir" (i), "m" (v->counter));
}
EXPORT_SYMBOL(atomic_sub);

//---------------------------- atomic_sub_and_test -----------------------------
int atomic_sub_and_test(int i, atomic_t *v)
{
   unsigned char c;

   __asm__ __volatile__(
      LOCK "subl %2,%0; sete %1"
      :"=m" (v->counter), "=qm" (c)
      :"ir" (i), "m" (v->counter) : "memory");
   return c;
}
EXPORT_SYMBOL(atomic_sub_and_test);

//--------------------------------- atomic_inc ---------------------------------
void atomic_inc(atomic_t *v)
{
   __asm__ __volatile__(
      LOCK "incl %0"
      :"=m" (v->counter)
      :"m" (v->counter));
}
EXPORT_SYMBOL(atomic_inc);

//--------------------------------- atomic_dec ---------------------------------
void atomic_dec(atomic_t *v)
{
   __asm__ __volatile__(
      LOCK "decl %0"
      :"=m" (v->counter)
      :"m" (v->counter));
}
EXPORT_SYMBOL(atomic_dec);

//---------------------------- atomic_dec_and_test -----------------------------
int atomic_dec_and_test(atomic_t *v)
{
   unsigned char c;

   __asm__ __volatile__(
      LOCK "decl %0; sete %1"
      :"=m" (v->counter), "=qm" (c)
      :"m" (v->counter) : "memory");
   return c != 0;
}
EXPORT_SYMBOL(atomic_dec_and_test);

//---------------------------- atomic_inc_and_test -----------------------------
int atomic_inc_and_test(atomic_t *v)
{
   unsigned char c;

   __asm__ __volatile__(
      LOCK "incl %0; sete %1"
      :"=m" (v->counter), "=qm" (c)
      :"m" (v->counter) : "memory");
   return c != 0;
}
EXPORT_SYMBOL(atomic_inc_and_test);

//---------------------------- atomic_add_negative -----------------------------
int atomic_add_negative(int i, atomic_t *v)
{
   unsigned char c;

   __asm__ __volatile__(
      LOCK "addl %2,%0; sets %1"
      :"=m" (v->counter), "=qm" (c)
      :"ir" (i), "m" (v->counter) : "memory");
   return c;
}
EXPORT_SYMBOL(atomic_add_negative);

// #if defined(TARGET_OS2) && defined(CONFIG_LX_OPTIMIZE_SIZE)

#endif
