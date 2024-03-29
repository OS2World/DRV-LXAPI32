#ifndef _ASMi386_PARAM_H
#define _ASMi386_PARAM_H

#ifdef __KERNEL__
#ifdef TARGET_OS2
# define HZ    310      /* Internal kernel timer frequency */
# define USER_HZ  100      /* .. some user interfaces are in "ticks" */
#else
# define HZ    1000     /* Internal kernel timer frequency */
# define USER_HZ  100      /* .. some user interfaces are in "ticks" */
#endif
# define CLOCKS_PER_SEC (USER_HZ)   /* like times() */
#endif

#ifndef HZ
#define HZ 100
#endif

#define EXEC_PAGESIZE   4096

#ifndef NOGROUP
#define NOGROUP      (-1)
#endif

#define MAXHOSTNAMELEN  64 /* max length of hostname */

#endif
