/* $Id: lxlibcwcalls.h,v 1.3 2005/07/11 22:18:30 smilcke Exp $ */

/*
 * lxlibcwcalls.h
 * Autor:               Stefan Milcke
 * Erstellt am:         14.04.2005
 * Letzte Aenderung am: 05.07.2005
 *
*/

#ifndef LXLIBCWCALLS_H_INCLUDED
#define LXLIBCWCALLS_H_INCLUDED

#define LXWCALLLINKAGE  static inline
#define LXWCATTRINLINE  __attribute__((always_inline))

LXWCALLLINKAGE void* LXWCATTRINLINE
  malloc(unsigned long size)
{
 void* rc;
 __asm__ __volatile__
 (
  "call  malloc\n\t"
  :"=a" ((unsigned long)rc)
  :"a" ((unsigned long)size)
  :"ecx", "edx", "memory"
 );
 return rc;
}

LXWCALLLINKAGE void* LXWCATTRINLINE
  realloc(void* ptr,unsigned long size)
{
 void* rc;
 __asm__ __volatile__
 (
  "call  realloc\n\t"
  :"=a"((unsigned long)rc)
  :"a"((unsigned long)ptr), "d"((unsigned long)size)
  : "ebx", "ecx", "memory"
 );
 return rc;
}

LXWCALLLINKAGE void LXWCATTRINLINE
  free(void* ptr)
{
 __asm__ __volatile__
 (
  "call  free\n\t"
  :
  :"a" ((unsigned long)ptr)
  : "ecx", "edx", "memory"
 );
}

LXWCALLLINKAGE void* LXWCATTRINLINE
  memccpy(void* m1,__const__ void* m2,int c,size_t size)
{
 void* rc;
 __asm__ __volatile__
 (
  "call  memccpy\n\t"
  :"=a" ((unsigned long)rc)
  :"a" ((unsigned long)m1), "d"((unsigned long)m2), "c"((unsigned long)c), "b"((unsigned long)size)
  : "memory"
 );
 return rc;
}

LXWCALLLINKAGE void* LXWCATTRINLINE
 memchr(__const__ void* m1,int c,size_t size)
{
 void* rc;
 __asm__ __volatile__
 (
  "call  memchr\n\t"
  :"=a" ((unsigned long)rc)
  :"a" ((unsigned long)m1), "d"((unsigned long)c), "c"((unsigned long)size)
  :"memory"
 );
 return rc;
}

LXWCALLLINKAGE int LXWCATTRINLINE
 memcmp(__const__ void* m1,__const__ void* m2,size_t size)
{
 int rc;
 __asm__ __volatile__
 (
  "call  memcmp\n\t"
  :"=a" ((unsigned long)rc)
  :"a" ((unsigned long)m1), "d"((unsigned long)m2), "c"((unsigned long)size)
  :"memory"
 );
 return rc;
}

LXWCALLLINKAGE void* LXWCATTRINLINE
  memcpy(void* m1,__const__ void* m2,size_t size)
{
 void* rc;
 __asm__ __volatile__
 (
  "call  memcpy\n\t"
  :"=a" ((unsigned long)rc)
  :"a" ((unsigned long)m1), "d"((unsigned long)m2), "c"((unsigned long)size)
  :"memory"
 );
 return rc;
}

LXWCALLLINKAGE void* LXWCATTRINLINE
  memmove(void* m1,__const__ void* m2,size_t size)
{
 void* rc;
 __asm__ __volatile__
 (
  "call  memmove\n\t"
  :"=a" ((unsigned long)rc)
  :"a" ((unsigned long)m1), "d"((unsigned long)m2), "c"((unsigned long)size)
  :"memory"
 );
 return rc;
}

LXWCALLLINKAGE void* LXWCATTRINLINE
  memset(void* m1,int c,size_t size)
{
 void* rc;
 __asm__ __volatile__
 (
  "call  memset\n\t"
  :"=a" ((unsigned long)rc)
  :"a" ((unsigned long)m1), "d"((unsigned long)c), "c"((unsigned long)size)
  :"memory"
 );
 return rc;
}

LXWCALLLINKAGE int LXWCATTRINLINE
  memicmp(__const__ void* m1,__const__ void* m2,size_t size)
{
 int rc;
 __asm__ __volatile__
 (
  "call  memicmp\n\t"
  :"=a" ((unsigned long)rc)
  :"a" ((unsigned long)m1), "d"((unsigned long)m2), "c"((unsigned long)size)
  :"memory"
 );
 return rc;
}

LXWCALLLINKAGE char* LXWCATTRINLINE
  strcat(char* s1,__const__ char* s2)
{
 char* rc;
 __asm__ __volatile__
 (
  "call  strcat\n\t"
  :"=a"((unsigned long)rc)
  :"a" ((unsigned long)s1), "d" ((unsigned long)s2)
  : "ecx", "memory"
 );
 return rc;
}

LXWCALLLINKAGE int LXWCATTRINLINE
  strcmp(__const__ char* s1,__const__ char* s2)
{
 int rc;
 __asm__ __volatile__
 (
  "call  strcmp\n\t"
  :"=a"((unsigned long)rc)
  :"a" ((unsigned long)s1), "d" ((unsigned long)s2)
  : "ecx", "memory"
 );
 return rc;
}

LXWCALLLINKAGE char* LXWCATTRINLINE
  strcpy(char* s1,const char* s2)
{
 char* rc;
 __asm__ __volatile__
 (
  "call  strcpy\n\t"
  :"=a"((unsigned long)rc)
  :"a" ((unsigned long)s1), "d" ((unsigned long)s2)
  : "ecx", "memory"
 );
 return rc;
}

LXWCALLLINKAGE int LXWCATTRINLINE
  strlen(const char* s)
{
 int rc;
 __asm__ __volatile__
 (
  "call  strlen\n\t"
  :"=a"((unsigned long)rc)
  :"a" ((unsigned long)s)
  :"ecx", "memory"
 );
 return rc;
}

LXWCALLLINKAGE char* LXWCATTRINLINE
  strncat(char* s1,__const__ char* s2,size_t size)
{
 char* rc;
 __asm__ __volatile__
 (
  "call  strncat\n\t"
  :"=a"((unsigned long)rc)
  :"a" ((unsigned long)s1), "d" ((unsigned long)s2), "c" ((unsigned long)size)
  : "memory"
 );
 return rc;
}

LXWCALLLINKAGE int LXWCATTRINLINE
  strncmp(__const__ char* s1,__const__ char* s2,size_t size)
{
 int rc;
 __asm__ __volatile__
 (
  "call  strncmp\n\t"
  :"=a"((unsigned long)rc)
  :"a" ((unsigned long)s1), "d" ((unsigned long)s2), "c" ((unsigned long)size)
  : "memory"
 );
 return rc;
}


LXWCALLLINKAGE char* LXWCATTRINLINE
  strncpy(char* s1,const char* s2,size_t size)
{
 char* rc;
 __asm__ __volatile__
 (
  "call  strncpy\n\t"
  :"=a"((unsigned long)rc)
  :"a" ((unsigned long)s1), "d" ((unsigned long)s2), "c" ((unsigned long)size)
  : "memory"
 );
 return rc;
}

LXWCALLLINKAGE char* LXWCATTRINLINE
  strchr(const char* s,int c)
{
 char* rc;
 __asm__ __volatile__
 (
  "call  strchr\n\t"
  :"=a"((unsigned long)rc)
  :"a" ((unsigned long)s), "d" ((unsigned long)c)
  :"ecx", "memory"
 );
 return rc;
}

#endif //LXLIBCWCALLS_H_INCLUDED
