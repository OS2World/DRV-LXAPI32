#ifndef _I386_BUG_H
#define _I386_BUG_H

#include <linux/config.h>

/*
 * Tell the user there is some problem.
 * The offending file and line are encoded after the "officially
 * undefined" opcode for parsing in the trap handler.
 */

#if 1 /* Set to zero for a slightly smaller kernel */
#ifdef TARGET_OS2
#define BUG() {printk("BUG %s in %s\n",(__LINE__),(__FILE__)); \
      DevInt3();}
#else
#define BUG()           \
 __asm__ __volatile__(  "ud2\n"     \
         "\t.word %c0\n"   \
         "\t.long %c1\n"   \
          : : "i" (__LINE__), "i" (__FILE__))
#endif
#else
#ifdef TARGET_OS2
#define BUG() {printk("BUG %s in %s\n",(__LINE__),(__FILE__)); \
      DevInt3();}
#else
#define BUG() __asm__ __volatile__("ud2\n")
#endif
#endif

#define BUG_ON(condition) do { if (unlikely((condition)!=0)) BUG(); } while(0)

#define PAGE_BUG(page) do { \
   BUG(); \
} while (0)

#define WARN_ON(condition) do { \
   if (unlikely((condition)!=0)) { \
      printk("Badness in %s at %s:%d\n", __FUNCTION__, __FILE__, __LINE__); \
      dump_stack(); \
   } \
} while (0)

#endif
