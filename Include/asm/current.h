#ifndef _I386_CURRENT_H
#define _I386_CURRENT_H

#include <linux/thread_info.h>

struct task_struct;

#if defined(TARGET_OS2) && defined(CONFIG_LX_OPTIMIZE_SIZE)
extern struct task_struct* get_current(void);
#else
static inline struct task_struct * get_current(void)
{
   return current_thread_info()->task;
}
#endif

#define current get_current()

#endif /* !(_I386_CURRENT_H) */
