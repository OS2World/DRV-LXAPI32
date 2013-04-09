/* $Id: oal_usercopy.c,v 1.6 2005/07/23 21:15:16 smilcke Exp $ */

/*
 * lal_usercopy.c
 * Autor:               Stefan Milcke
 * Erstellt am:         28.06.2004
 * Letzte Aenderung am: 18.07.2005
 *
*/

#include <lxcommon.h>

/*
 * User address space access functions.
 * The non inlined parts of asm-i386/uaccess.h are here.
 *
 * Copyright 1997 Andi Kleen <ak@muc.de>
 * Copyright 1997 Linus Torvalds
 */
#include <linux/config.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/blkdev.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <asm/mmx.h>

//--------------------------------- clear_user ---------------------------------
unsigned long clear_user(void __user *to,unsigned long n)
{
// if(!DevVerifyAccess((ULONG)to,n,1))
 {
  char hLock[12];
  ULONG pgCount=0;
  if(!DevVMLock(VMDHL_WRITE | VMDHL_LONG,(ULONG)to,n,(LINEAR)-1,hLock,(LINEAR)&pgCount))
  {
   memset(to,0,n);
   DevVMUnLock(hLock);
  }
 }
 return n;
}

//----------------------------- __copy_to_user_ll ------------------------------
unsigned long __copy_to_user_ll(void __user* to,const void *from
                                ,unsigned long n)
{
// if(!DevVerifyAccess((ULONG)to,n,1))
 {
  char hLock[12];
  ULONG pgCount=0;
  if(!DevVMLock(VMDHL_WRITE | VMDHL_LONG,(ULONG)to,n,(LINEAR)-1,hLock,(LINEAR)&pgCount))
  {
   memcpy(to,from,n);
   DevVMUnLock(hLock);
  }
 }
// return n;
 return 0;
}

//---------------------------- __copy_from_user_ll -----------------------------
unsigned long __copy_from_user_ll(void* to,const void __user* from
                                  ,unsigned long n)
{
// if(!DevVerifyAccess((ULONG)from,n,0))
 {
  char hLock[12];
  ULONG pgCount=0;
  if(!DevVMLock(VMDHL_LONG,(ULONG)from,n,(LINEAR)-1,hLock,(LINEAR)&pgCount))
  {
   memcpy(to,from,n);
   DevVMUnLock(hLock);
  }
 }
 return n;
}

#ifdef TARGET_OS2
#define __do_strncpy_from_user(dst,src,count,res)  \
do {                                               \
 int __d0, __d1, __d2;                             \
 __asm__ __volatile__(                             \
   "     testl %1,%1\n"                            \
   "     jz    2f\n"                               \
   "0:   lodsb\n"                                  \
   "     stosb\n"                                  \
   "     testb %%al,%%al\n"                        \
   "     jz    1f\n"                               \
   "     decl  %1\n"                               \
   "     jnz   0b\n"                               \
   "1:   subl  %1,%0\n"                            \
   "2:\n"                                          \
   : "=d"(res), "=c"(count), "=&a" (__d0),         \
     "=&S" (__d1), "=&D" (__d2)                    \
   : "i" (-EFAULT), "0"(count), "1"(count), "3"(src), "4"(dst) \
   : "memory");                                    \
} while(0)
#else
#define __do_strncpy_from_user(dst,src,count,res)           \
do {                             \
   int __d0, __d1, __d2;                     \
   __asm__ __volatile__(                     \
      "  testl %1,%1\n"                \
      "  jz 2f\n"                \
      "0:   lodsb\n"                \
      "  stosb\n"                \
      "  testb %%al,%%al\n"               \
      "  jz 1f\n"                \
      "  decl %1\n"                 \
      "  jnz 0b\n"                  \
      "1:   subl %1,%0\n"                 \
      "2:\n"                        \
      ".section .fixup,\"ax\"\n"             \
      "3:   movl %5,%0\n"                 \
      "  jmp 2b\n"                  \
      ".previous\n"                    \
      ".section __ex_table,\"a\"\n"             \
      "  .align 4\n"                \
      "  .long 0b,3b\n"                \
      ".previous"                   \
      : "=d"(res), "=c"(count), "=&a" (__d0), "=&S" (__d1),    \
        "=&D" (__d2)                   \
      : "i"(-EFAULT), "0"(count), "1"(count), "3"(src), "4"(dst) \
      : "memory");                     \
} while (0)
#endif

//----------------------------- strncpy_from_user ------------------------------
long strncpy_from_user(char* dst,const char __user* src,long count)
{
 long res=0;
 char hLock[12];
 ULONG pgCount=0;
 if(!DevVMLock(VMDHL_LONG,(ULONG)src,1,(LINEAR)-1,hLock,(LINEAR)&pgCount))
 {
  __do_strncpy_from_user(dst,src,count,res);
  DevVMUnLock(hLock);
 }
 return res;
}

//-------------------------------- copy_to_user --------------------------------
unsigned long copy_to_user(void *to, const void *from, unsigned long n)
{
 if(to == NULL || from == NULL)
  return -EFAULT;
 if(n == 0)
  return 0;
// if(!DevVerifyAccess((ULONG)to,n,1))
 {
  char hLock[12];
  ULONG pgCount=0;
  if(!DevVMLock(VMDHL_WRITE | VMDHL_LONG,(ULONG)to,n,(LINEAR)-1,hLock,(LINEAR)&pgCount))
  {
   memcpy(to, from, n);
   DevVMUnLock(hLock);
  }
 }
 return 0;
}

//------------------------------- copy_from_user -------------------------------
unsigned long copy_from_user(void *to, const void *from, unsigned long n)
{
 if(to == NULL || from == NULL)
  return -EFAULT;
 if(n == 0)
  return 0;
// if(!DevVerifyAccess((ULONG)from,n,0))
 {
  char hLock[12];
  ULONG pgCount=0;
  if(!DevVMLock(VMDHL_LONG,(ULONG)from,n,(LINEAR)-1,hLock,(LINEAR)&pgCount))
  {
   memcpy(to, from, n);
   DevVMUnLock(hLock);
  }
 }
 return 0;
}

//-------------------------------- strnlen_user --------------------------------
long strnlen_user(const char __user *s,long n)
{
 long rc=0;
// if(!DevVerifyAccess((ULONG)s,n,0))
 {
  char hLock[12];
  ULONG pgCount=0;
  if(!DevVMLock(VMDHL_LONG,(ULONG)s,1,(LINEAR)-1,hLock,(LINEAR)&pgCount))
  {
   rc=strlen(s);
   DevVMUnLock(hLock);
  }
 }
 return rc;
}
