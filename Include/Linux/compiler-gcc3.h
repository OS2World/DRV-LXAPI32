/* Never include this file directly.  Include <linux/compiler.h> instead.  */

/* These definitions are for GCC v3.x.  */
#include <linux/compiler-gcc.h>

#if __GNUC_MINOR__ >= 1  && __GNUC_MINOR__ < 4
# define inline      __inline__ __attribute__((always_inline))
# define __inline__  __inline__ __attribute__((always_inline))
# define __inline __inline__ __attribute__((always_inline))
#endif

#if __GNUC_MINOR__ > 0
# define __deprecated   __attribute__((deprecated))
#endif

#ifdef TARGET_OS2
#include <sys/cdefs.h>
#else // TARGET_OS2
# if __GNUC_MINOR__ >= 3
#  define __attribute_used__   __attribute__((__used__))
# else
#  define __attribute_used__   __attribute__((__unused__))
# endif
#endif // TARGET_OS2


#ifdef TARGET_OS2
#else // TARGET_OS2
# define __attribute_pure__   __attribute__((pure))
#endif // TARGET_OS2
#define __attribute_const__   __attribute__((__const__))

#if __GNUC_MINOR__ >= 1
#define  noinline __attribute__((noinline))
#endif
