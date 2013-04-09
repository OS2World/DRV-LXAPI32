/* $Id: lxlibcsysc.h,v 1.4 2005/06/16 22:44:21 smilcke Exp $ */

/*
 * lxlibcsysc.h
 * Autor:               Stefan Milcke
 * Erstellt am:         19.03.2005
 * Letzte Aenderung am: 16.06.2005
 *
*/

#ifndef LXLIBCSYSC_H_INCLUDED
#define LXLIBCSYSC_H_INCLUDED

#include <lxlibcdefs.h>

#ifndef NO_INCL_LXUTL_IOCTL
#include <lxuapioctl.h>
#endif

extern LXAPISYSCRET LXAPIENTRY syscall0(int num);
extern LXAPISYSCRET LXAPIENTRY syscall1(int num
                                        ,unsigned long a1);
extern LXAPISYSCRET LXAPIENTRY syscall2(int num
                                        ,unsigned long a1
                                        ,unsigned long a2);
extern LXAPISYSCRET LXAPIENTRY syscall3(int num
                                        ,unsigned long a1
                                        ,unsigned long a2
                                        ,unsigned long a3);
extern LXAPISYSCRET LXAPIENTRY syscall4(int num
                                        ,unsigned long a1
                                        ,unsigned long a2
                                        ,unsigned long a3
                                        ,unsigned long a4);
extern LXAPISYSCRET LXAPIENTRY syscall5(int num
                                        ,unsigned long a1
                                        ,unsigned long a2
                                        ,unsigned long a3
                                        ,unsigned long a4
                                        ,unsigned long a5);
extern LXAPISYSCRET LXAPIENTRY syscall6(int num
                                        ,unsigned long a1
                                        ,unsigned long a2
                                        ,unsigned long a3
                                        ,unsigned long a4
                                        ,unsigned long a5
                                        ,unsigned long a6);

#define SYSCALL0(nr) syscall0((nr))
#define SYSCALL1(nr,a1) syscall1((nr),(unsigned long)(a1))
#define SYSCALL2(nr,a1,a2)                \
   syscall2((nr),(unsigned long)(a1)      \
                ,(unsigned long)(a2))
#define SYSCALL3(nr,a1,a2,a3)             \
   syscall3((nr),(unsigned long)(a1)      \
                ,(unsigned long)(a2)      \
                ,(unsigned long)(a3))

#define SYSCALL4(nr,a1,a2,a3,a4)          \
   syscall4((nr),(unsigned long)(a1)      \
                ,(unsigned long)(a2)      \
                ,(unsigned long)(a3)      \
                ,(unsigned long)(a4))

#define SYSCALL5(nr,a1,a2,a3,a4,a5)       \
   syscall5((nr),(unsigned long)(a1)      \
                ,(unsigned long)(a2)      \
                ,(unsigned long)(a3)      \
                ,(unsigned long)(a4)      \
                ,(unsigned long)(a5))

#define SYSCALL6(nr,a1,a2,a3,a4,a5,a6)    \
   syscall6((nr),(unsigned long)(a1)      \
                ,(unsigned long)(a2)      \
                ,(unsigned long)(a3)      \
                ,(unsigned long)(a4)      \
                ,(unsigned long)(a5)      \
                ,(unsigned long)(a6))


#endif //LXLIBCSYSC_H_INCLUDED
