#ifndef __LINUX_SMPLOCK_H
#define __LINUX_SMPLOCK_H

#include <linux/config.h>
#include <linux/sched.h>
#include <linux/spinlock.h>

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)

extern spinlock_t kernel_flag;

#define kernel_locked()    (current->lock_depth >= 0)

#define get_kernel_lock()  spin_lock(&kernel_flag)
#define put_kernel_lock()  spin_unlock(&kernel_flag)

/*
 * Release global kernel lock.
 */
#if defined(TARGET_OS2) && defined(CONFIG_LX_OPTIMIZE_SIZE)
extern void release_kernel_lock(struct task_struct *task);
#else
static inline void release_kernel_lock(struct task_struct *task)
{
   if (unlikely(task->lock_depth >= 0))
      put_kernel_lock();
}
#endif

/*
 * Re-acquire the kernel lock
 */
#if defined(TARGET_OS2) && defined(CONFIG_LX_OPTIMIZE_SIZE)
extern void reacquire_kernel_lock(struct task_struct *task);
#else
static inline void reacquire_kernel_lock(struct task_struct *task)
{
   if (unlikely(task->lock_depth >= 0))
      get_kernel_lock();
}
#endif

/*
 * Getting the big kernel lock.
 *
 * This cannot happen asynchronously,
 * so we only need to worry about other
 * CPU's.
 */
#if defined(TARGET_OS2) && defined(CONFIG_LX_OPTIMIZE_SIZE)
extern void lock_kernel(void);
#else
static inline void lock_kernel(void)
{
   int depth = current->lock_depth+1;
   if (likely(!depth))
      get_kernel_lock();
   current->lock_depth = depth;
}
#endif

#if defined(TARGET_OS2) && defined(CONFIG_LX_OPTIMIZE_SIZE)
extern void unlock_kernel(void);
#else
static inline void unlock_kernel(void)
{
   BUG_ON(current->lock_depth < 0);
   if (likely(--current->lock_depth < 0))
      put_kernel_lock();
}
#endif

#else

#define lock_kernel()            do { } while(0)
#define unlock_kernel()          do { } while(0)
#define release_kernel_lock(task)      do { } while(0)
#define reacquire_kernel_lock(task)    do { } while(0)
#define kernel_locked()          1

#endif /* CONFIG_SMP || CONFIG_PREEMPT */
#endif /* __LINUX_SMPLOCK_H */
