/* string.h,v 1.13 2004/09/14 22:27:36 bird Exp */
/** @file
 * FreeBSD 5.1
 * @changed bird: EMXifications.
 * @changed bird: GNU extension(s).
 */

/*-
 * Copyright (c) 1990, 1993
 * The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * This product includes software developed by the University of
 * California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @(#)string.h   8.1 (Berkeley) 6/2/93
 * $FreeBSD: src/include/string.h,v 1.18 2002/10/14 20:38:40 mike Exp $
 */

#ifndef _STRING_H_
#define  _STRING_H_

#include <sys/cdefs.h>
#include <sys/_types.h>

/*
 * Prototype functions which were historically defined in <string.h>, but
 * are required by POSIX to be prototyped in <strings.h>.
 */
#if __BSD_VISIBLE
#include <strings.h>
#endif

#if !defined(_SIZE_T_DECLARED) && !defined(_SIZE_T)  /* bird: EMX */
typedef  __size_t size_t;
#define  _SIZE_T_DECLARED
#define _SIZE_T                         /* bird: EMX */
#endif

#ifndef  NULL
#define  NULL  0
#endif

#if (defined (LXLIBC) || defined (LXUTIL))
#include <lxlibcwcalls.h>
#else
__BEGIN_DECLS
#if __POSIX_VISIBLE >= 200112 || __XSI_VISIBLE
void  *memccpy(void * __restrict, const void * __restrict, int, size_t);
#endif
void  *memchr(const void *, int, size_t);
int    memcmp(const void *, const void *, size_t);
void  *memcpy(void * __restrict, const void * __restrict, size_t);
void  *memmove(void *, const void *, size_t);
void  *memset(void *, int, size_t);
#if __BSD_VISIBLE
char  *stpcpy(char *, const char *);
char  *strcasestr(const char *, const char *);
#endif
char  *strcat(char * __restrict, const char * __restrict);
char  *strchr(const char *, int);
int    strcmp(const char *, const char *);
int    strcoll(const char *, const char *);
char  *strcpy(char * __restrict, const char * __restrict);
size_t    strcspn(const char *, const char *);
#if __POSIX_VISIBLE >= 200112 || __XSI_VISIBLE
char  *strdup(const char *);
#endif
char  *strerror(int);
#if __POSIX_VISIBLE >= 200112
int    strerror_r(int, char *, size_t);
#endif
#if __BSD_VISIBLE
size_t    strlcat(char *, const char *, size_t);
size_t    strlcpy(char *, const char *, size_t);
#endif
size_t    strlen(const char *);
#if __BSD_VISIBLE
void   strmode(int, char *);
#endif
char  *strncat(char * __restrict, const char * __restrict, size_t);
int    strncmp(const char *, const char *, size_t);
char  *strncpy(char * __restrict, const char * __restrict, size_t);
#if __BSD_VISIBLE
char  *strnstr(const char *, const char *, size_t);
#endif
char  *strpbrk(const char *, const char *);
char  *strrchr(const char *, int);
#if __BSD_VISIBLE
char  *strsep(char **, const char *);
 char *strsignal(int);
#endif
size_t    strspn(const char *, const char *);
char  *strstr(const char *, const char *);
char  *strtok(char * __restrict, const char * __restrict);
#if __POSIX_VISIBLE >= 199506 || __XSI_VISIBLE >= 500
char  *strtok_r(char *, const char *, char **);
#endif
size_t    strxfrm(char * __restrict, const char * __restrict, size_t);
#if __BSD_VISIBLE
void   swab(const void *, void *, size_t);
#endif


/* bird: EMX stuff - start */

#if !defined (__STRICT_ANSI__) && !defined (_POSIX_SOURCE) || defined(__USE_EMX)
#if !defined (_MEMDIF_EQ)
#define _MEMDIF_EQ 0xffffffff
#endif
int     memicmp(__const__ void *, __const__ void *, size_t);
int     stricmp(__const__ char *, __const__ char *);
char *  strlwr(char *);
int     strnicmp(__const__ char *, __const__ char *, size_t);
char *  strnset(char *, int, size_t);
char *  strrev(char *);
char *  strset(char *, int);
char *  strupr(char *);
#endif

#if (!defined (__STRICT_ANSI__) && !defined (_POSIX_SOURCE)) || defined (_WITH_UNDERSCORE) || defined(__USE_EMX)
size_t _memcount (__const__ void *, int, size_t);
size_t _memdif (__const__ void *, __const__ void *, size_t);
void *_memrchr (const void *, int, size_t);
void _memswap (void *, void *, size_t);
char *_strncpy (char *, __const__ char *, size_t);
void *_memccpy (void *, __const__ void *, int, size_t);
int _memicmp (__const__ void *, __const__ void *, size_t);
char *_strdup (__const__ char *);
int _stricmp (__const__ char *, __const__ char *);
char *_strlwr (char *);
int _strnicmp (__const__ char *, __const__ char *, size_t);
char *_strnset (char *, int, size_t);
char *_strrev (char *);
char *_strset (char *, int);
char *_strupr (char *);
char *_strsep (char **, __const__ char *);
#endif

/* bird: EMX stuff - end */

/* bird: GNU stuff - start */
#ifdef __USE_GNU
char  *basename(const char *);
size_t   strnlen(const char *, size_t);
void    *mempcpy(void *, const void *, size_t);
char    *strndup(const char *, size_t);
void    *memrchr(const void *, int, size_t);
#define  memrchr(pach, ch, cch)     _memrchr(pach, ch, cch)
int      strverscmp(const char *, const char *);

size_t   __strnlen(const char *, size_t);
void    *__mempcpy(void *, const void *, size_t);
char    *__strndup(const char *, size_t);
void    *_memrchr(const void *, int, size_t);
void    *__memrchr(const void *, int, size_t);
#define  __memrchr(pach, ch, cch)   _memrchr(pach, ch, cch)
int      __strverscmp(const char *, const char *);

#endif
/* bird: GNU stuff - end */

__END_DECLS

#endif // LXLIBC || LXUTIL

#endif /* _STRING_H_ */

