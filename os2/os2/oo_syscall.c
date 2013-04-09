/* $Id: oo_syscall.c,v 1.1 2005/02/06 22:49:35 smilcke Exp $ */

/*
 * oo_syscall.c
 * Autor:               Stefan Milcke
 * Erstellt am:         30.01.2005
 * Letzte Aenderung am: 30.01.2005
 *
*/

#include <lxcommon.h>

// Undefined syscalls
#define LX_UNDEF_SYSCALL(name) \
 void name(void) \
 {NOT_IMPLEMENTED();}

LX_UNDEF_SYSCALL(sys_mlock);
LX_UNDEF_SYSCALL(sys_uname);
LX_UNDEF_SYSCALL(sys_lookup_dcookie);
LX_UNDEF_SYSCALL(sys_msync);
LX_UNDEF_SYSCALL(sys_mq_timedreceive);
LX_UNDEF_SYSCALL(sys_chown16);
LX_UNDEF_SYSCALL(old_select);
LX_UNDEF_SYSCALL(sys_ptrace);
LX_UNDEF_SYSCALL(sys_vm86old);
LX_UNDEF_SYSCALL(sys_getgid16);
LX_UNDEF_SYSCALL(sys_getuid16);
LX_UNDEF_SYSCALL(sys_ioperm);
LX_UNDEF_SYSCALL(sys_mremap);
LX_UNDEF_SYSCALL(sys_setgid16);
LX_UNDEF_SYSCALL(sys_madvise);
LX_UNDEF_SYSCALL(sys_setuid16);
LX_UNDEF_SYSCALL(sys_syslog);
LX_UNDEF_SYSCALL(sys_fchown16);
LX_UNDEF_SYSCALL(sys_mincore);
LX_UNDEF_SYSCALL(sys_munlock);
LX_UNDEF_SYSCALL(sys_mq_open);
LX_UNDEF_SYSCALL(sys_swapon);
LX_UNDEF_SYSCALL(sys_lchown16);
LX_UNDEF_SYSCALL(sys_swapoff);
LX_UNDEF_SYSCALL(sys_sysinfo);
LX_UNDEF_SYSCALL(sys_mlockall);
LX_UNDEF_SYSCALL(sys_olduname);
LX_UNDEF_SYSCALL(sys_getegid16);
LX_UNDEF_SYSCALL(sys_quotactl);
LX_UNDEF_SYSCALL(sys_geteuid16);
LX_UNDEF_SYSCALL(sys_mprotect);
LX_UNDEF_SYSCALL(sys_fadvise64);
LX_UNDEF_SYSCALL(sys_sigaction);
LX_UNDEF_SYSCALL(sys_sigreturn);
LX_UNDEF_SYSCALL(sys_mq_unlink);
LX_UNDEF_SYSCALL(sys_setregid16);
LX_UNDEF_SYSCALL(sys_setfsgid16);
LX_UNDEF_SYSCALL(sys_epoll_ctl);
LX_UNDEF_SYSCALL(sys_mq_notify);
LX_UNDEF_SYSCALL(sys_setreuid16);
LX_UNDEF_SYSCALL(sys_setfsuid16);
LX_UNDEF_SYSCALL(sys_getresgid16);
LX_UNDEF_SYSCALL(sys_getresuid16);
LX_UNDEF_SYSCALL(sys_setresgid16);
LX_UNDEF_SYSCALL(sys_setresuid16);
LX_UNDEF_SYSCALL(sys_getgroups16);
LX_UNDEF_SYSCALL(sys_setgroups16);
LX_UNDEF_SYSCALL(sys_fadvise64_64);
LX_UNDEF_SYSCALL(sys_munlockall);
LX_UNDEF_SYSCALL(sys_nfsservctl);
LX_UNDEF_SYSCALL(sys_sigsuspend);
LX_UNDEF_SYSCALL(sys_socketcall);
LX_UNDEF_SYSCALL(sys_sigaltstack);
LX_UNDEF_SYSCALL(sys_epoll_wait);
LX_UNDEF_SYSCALL(sys_mq_timedsend);
LX_UNDEF_SYSCALL(sys_epoll_create);
LX_UNDEF_SYSCALL(sys_rt_sigreturn);
LX_UNDEF_SYSCALL(sys_mq_getsetattr);
LX_UNDEF_SYSCALL(sys_rt_sigsuspend);
LX_UNDEF_SYSCALL(sys_ipc);
LX_UNDEF_SYSCALL(old_mmap);
LX_UNDEF_SYSCALL(sys_vm86);
LX_UNDEF_SYSCALL(sys_acct);
LX_UNDEF_SYSCALL(sys_pipe);
LX_UNDEF_SYSCALL(sys_iopl);
LX_UNDEF_SYSCALL(sys_mmap2);
LX_UNDEF_SYSCALL(sys_alarm);
