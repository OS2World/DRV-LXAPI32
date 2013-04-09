/* $Id: oak_entry.s,v 1.5 2005/12/26 23:42:04 smilcke Exp $ */

/*
 * entry.S
 * Autor:               Stefan Milcke
 * Erstellt am:         23.09.2004
 * Letzte Aenderung am: 31.01.2005
 *
*/
/*
 *  linux/arch/i386/entry.S
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * entry.S contains the system-call and fault low-level handling routines.
 * This also contains the timer-interrupt handler, as well as all interrupts
 * and faults that can result in a task-switch.
 *
 * NOTE: This code handles signal-recognition, which happens every time
 * after a timer-interrupt and after each system call.
 *
 * I changed all the .align's to 4 (16 byte alignment), as that's faster
 * on a 486.
 *
 * Stack layout in 'ret_from_system_call':
 * 	ptrace needs to have all regs on the stack.
 *	if the order here is changed, it needs to be
 *	updated in fork.c:copy_process, signal.c:do_signal,
 *	ptrace.c and ptrace.h
 *
 *	 0(%esp) - %ebx
 *	 4(%esp) - %ecx
 *	 8(%esp) - %edx
 *       C(%esp) - %esi
 *	10(%esp) - %edi
 *	14(%esp) - %ebp
 *	18(%esp) - %eax
 *	1C(%esp) - %ds
 *	20(%esp) - %es
 *	24(%esp) - orig_eax
 *	28(%esp) - %eip
 *	2C(%esp) - %cs
 *	30(%esp) - %eflags
 *	34(%esp) - %oldesp
 *	38(%esp) - %oldss
 *
 * "current" is in register %ebx during any slow entries.
 */

#include <linux/config.h>
#include <linux/linkage.h>
#include <asm/thread_info.h>
#include <asm/errno.h>
#include <asm/segment.h>
#include <asm/smp.h>
#include <asm/page.h>
#include "irq_vectors.h"

EBX		= 0x00
ECX		= 0x04
EDX		= 0x08
ESI		= 0x0C
EDI		= 0x10
EBP		= 0x14
EAX		= 0x18
DS		= 0x1C
ES		= 0x20
ORIG_EAX	= 0x24
EIP		= 0x28
CS		= 0x2C
EFLAGS		= 0x30
OLDESP		= 0x34
OLDSS		= 0x38

CF_MASK		= 0x00000001
TF_MASK		= 0x00000100
IF_MASK		= 0x00000200
DF_MASK		= 0x00000400
NT_MASK		= 0x00004000
VM_MASK		= 0x00020000

/*
 * ESP0 is at offset 4. 0x200 is the size of the TSS, and
 * also thus the top-of-stack pointer offset of SYSENTER_ESP
 */
TSS_ESP0_OFFSET = (4 - 0x200)

#ifdef CONFIG_PREEMPT
#define preempt_stop		cli
#else
#define preempt_stop
#define resume_kernel		restore_all
#endif

#ifdef TARGET_OS2
#define SAVE_ALL \
	cld; \
	pushl %es; \
	pushl %ds; \
	pushl %eax; \
	pushl %ebp; \
	pushl %edi; \
	pushl %esi; \
	pushl %edx; \
	pushl %ecx; \
	pushl %ebx;
// Don't update ES and DS!!!
#else
#define SAVE_ALL \
	cld; \
	pushl %es; \
	pushl %ds; \
	pushl %eax; \
	pushl %ebp; \
	pushl %edi; \
	pushl %esi; \
	pushl %edx; \
	pushl %ecx; \
	pushl %ebx; \
	movl $(__USER_DS), %edx; \
	movl %edx, %ds; \
	movl %edx, %es;
#endif

#define RESTORE_INT_REGS \
	popl %ebx;	\
	popl %ecx;	\
	popl %edx;	\
	popl %esi;	\
	popl %edi;	\
	popl %ebp;	\
	popl %eax

#ifdef TARGET_OS2
#define RESTORE_REGS	\
	RESTORE_INT_REGS; \
	addl	$4, %esp;
// Don't update ES and DS!!!
#else
#define RESTORE_REGS	\
	RESTORE_INT_REGS; \
1:	popl %ds;	\
2:	popl %es;	\
.section .fixup,"ax";	\
3:	movl $0,(%esp);	\
	jmp 1b;		\
4:	movl $0,(%esp);	\
	jmp 2b;		\
.previous;		\
.section __ex_table,"a";\
	.align 4;	\
	.long 1b,3b;	\
	.long 2b,4b;	\
.previous
#endif

#ifdef TARGET_OS2
#define RESTORE_ALL	\
	RESTORE_REGS	\
	addl $8, %esp;	\
	sti;	\
	ret
#else
#define RESTORE_ALL	\
	RESTORE_REGS	\
	addl $4, %esp;	\
1:	iret;		\
.section .fixup,"ax";   \
2:	sti;		\
	movl $(__USER_DS), %edx; \
	movl %edx, %ds; \
	movl %edx, %es; \
	pushl $11;	\
	call do_exit;	\
.previous;		\
.section __ex_table,"a";\
	.align 4;	\
	.long 1b,2b;	\
.previous
#endif

.text
//#ifdef TARGET_OS2_GNU2
ENTRY(_ret_from_fork)
//#endif
ENTRY(ret_from_fork)
	addl	$4,%esp		// Stack pointer correction from _LXA_TaskStub32 (call ecx)
	pushl	%eax
	call	_schedule_tail
	GET_THREAD_INFO(%ebp)
	popl	%eax
	jmp		syscall_exit

/*
 * Return to user mode is not as complex as all this looks,
 * but we want the default path for a system call return to
 * go as quickly as possible which is why some of this is
 * less clear than it otherwise should be.
 */

	# userspace resumption stub bypassing syscall exit tracing
	ALIGN
ret_from_exception:
	preempt_stop
ret_from_intr:
	GET_THREAD_INFO(%ebp)
	movl EFLAGS(%esp), %eax		# mix EFLAGS and CS
	movb CS(%esp), %al
	testl $(VM_MASK | 3), %eax
	jz resume_kernel		# returning to kernel or vm86-space
ENTRY(resume_userspace)
 	cli				# make sure we don't miss an interrupt
					# setting need_resched or sigpending
					# between sampling and the iret
	movl TI_FLAGS(%ebp), %ecx
	andl $_TIF_WORK_MASK, %ecx	# is there any work to be done on
					# int/exception return?
	jne work_pending
	jmp restore_all

#ifdef CONFIG_PREEMPT
ENTRY(resume_kernel)
	cmpl $0,TI_PRE_COUNT(%ebp)	# non-zero preempt_count ?
	jnz restore_all
need_resched:
	movl TI_FLAGS(%ebp), %ecx	# need_resched set ?
	testb $_TIF_NEED_RESCHED, %cl
	jz restore_all
	testl $IF_MASK,EFLAGS(%esp)     # interrupts off (exception path) ?
	jz restore_all
	movl $PREEMPT_ACTIVE,TI_PRE_COUNT(%ebp)
	sti
	call schedule
	movl $0,TI_PRE_COUNT(%ebp)
	cli
	jmp need_resched
#endif

#ifdef TARGET_OS2
ENTRY(LX_do_system_call)
	jmp	system_call

// TARGET_OS2_GNU2
ENTRY(_LX_do_system_call)
	jmp	system_call
#endif

ENTRY(system_call)
	pushl %eax			# save orig_eax
	SAVE_ALL
	GET_THREAD_INFO(%ebp)
	cmpl $(nr_syscalls), %eax
	jae syscall_badsys
					# system call tracing in operation
	testb $(_TIF_SYSCALL_TRACE|_TIF_SYSCALL_AUDIT),TI_FLAGS(%ebp)
	jnz syscall_trace_entry
syscall_call:
	call *_sys_call_table(,%eax,4)
	movl %eax,EAX(%esp)		# store the return value
syscall_exit:
	cli				# make sure we don't miss an interrupt
					# setting need_resched or sigpending
					# between sampling and the iret
	movl TI_FLAGS(%ebp), %ecx
	testw $_TIF_ALLWORK_MASK, %cx	# current->work
	jne syscall_exit_work
restore_all:
	RESTORE_ALL

	# perform work that needs to be done immediately before resumption
	ALIGN
work_pending:
	testb $_TIF_NEED_RESCHED, %cl
	jz work_notifysig
work_resched:
	call _schedule
	cli				# make sure we don't miss an interrupt
					# setting need_resched or sigpending
					# between sampling and the iret
	movl TI_FLAGS(%ebp), %ecx
	andl $_TIF_WORK_MASK, %ecx	# is there any work to be done other
					# than syscall tracing?
	jz restore_all
	testb $_TIF_NEED_RESCHED, %cl
	jnz work_resched

work_notifysig:				# deal with pending signals and
					# notify-resume requests
	testl $VM_MASK, EFLAGS(%esp)
	movl %esp, %eax
	jne work_notifysig_v86		# returning to kernel-space or
					# vm86-space
	xorl %edx, %edx
	call _do_notify_resume
	jmp restore_all

	ALIGN
work_notifysig_v86:
	pushl %ecx
	call _save_v86_state
	popl %ecx
	movl %eax, %esp
	xorl %edx, %edx
	call _do_notify_resume
	jmp restore_all

	# perform syscall exit tracing
	ALIGN
syscall_trace_entry:
	movl $-ENOSYS,EAX(%esp)
	movl %esp, %eax
	xorl %edx,%edx
	call _do_syscall_trace
	movl ORIG_EAX(%esp), %eax
	cmpl $(nr_syscalls), %eax
	jnae syscall_call
	jmp syscall_exit

	# perform syscall exit tracing
	ALIGN
syscall_exit_work:
	testb $(_TIF_SYSCALL_TRACE|_TIF_SYSCALL_AUDIT), %cl
	jz work_pending
	sti				# could let do_syscall_trace() call
					# schedule() instead
	movl %esp, %eax
	movl $1, %edx
	call _do_syscall_trace
	jmp resume_userspace

	ALIGN
syscall_fault:
	pushl %eax			# save orig_eax
	SAVE_ALL
	GET_THREAD_INFO(%ebp)
	movl $-EFAULT,EAX(%esp)
	jmp resume_userspace

	ALIGN
syscall_badsys:
	movl $-ENOSYS,EAX(%esp)
	jmp resume_userspace

.data
ENTRY(_interrupt)
.text

vector=0
ENTRY(irq_entries_start)
.rept NR_IRQS
	ALIGN
1:	pushl	$vector-256
	jmp	_do_IRQ
.data
	.long	1b
.text
vector=vector+1
.endr

.data
ENTRY(_sys_call_table)
	.long _sys_restart_syscall	/* 0 - old "setup()" system call, used for restarting */
	.long _sys_exit
	.long _sys_fork
	.long _sys_read
	.long _sys_write
	.long _sys_open		/* 5 */
	.long _sys_close
	.long _sys_waitpid
	.long _sys_creat
	.long _sys_link
	.long _sys_unlink	/* 10 */
	.long _sys_execve
	.long _sys_chdir
	.long _sys_time
	.long _sys_mknod
	.long _sys_chmod		/* 15 */
	.long _sys_lchown16
	.long _sys_ni_syscall	/* old break syscall holder */
	.long _sys_stat
	.long _sys_lseek
	.long _sys_getpid	/* 20 */
	.long _sys_mount
	.long _sys_oldumount
	.long _sys_setuid16
	.long _sys_getuid16
	.long _sys_stime		/* 25 */
	.long _sys_ptrace
	.long _sys_alarm
	.long _sys_fstat
	.long _sys_pause
	.long _sys_utime		/* 30 */
	.long _sys_ni_syscall	/* old stty syscall holder */
	.long _sys_ni_syscall	/* old gtty syscall holder */
	.long _sys_access
	.long _sys_nice
	.long _sys_ni_syscall	/* 35 - old ftime syscall holder */
	.long _sys_sync
	.long _sys_kill
	.long _sys_rename
	.long _sys_mkdir
	.long _sys_rmdir		/* 40 */
	.long _sys_dup
	.long _sys_pipe
	.long _sys_times
	.long _sys_ni_syscall	/* old prof syscall holder */
	.long _sys_brk		/* 45 */
	.long _sys_setgid16
	.long _sys_getgid16
	.long _sys_signal
	.long _sys_geteuid16
	.long _sys_getegid16	/* 50 */
	.long _sys_acct
	.long _sys_umount	/* recycled never used phys() */
	.long _sys_ni_syscall	/* old lock syscall holder */
	.long _sys_ioctl
	.long _sys_fcntl		/* 55 */
	.long _sys_ni_syscall	/* old mpx syscall holder */
	.long _sys_setpgid
	.long _sys_ni_syscall	/* old ulimit syscall holder */
	.long _sys_olduname
	.long _sys_umask		/* 60 */
	.long _sys_chroot
	.long _sys_ustat
	.long _sys_dup2
	.long _sys_getppid
	.long _sys_getpgrp	/* 65 */
	.long _sys_setsid
	.long _sys_sigaction
	.long _sys_sgetmask
	.long _sys_ssetmask
	.long _sys_setreuid16	/* 70 */
	.long _sys_setregid16
	.long _sys_sigsuspend
	.long _sys_sigpending
	.long _sys_sethostname
	.long _sys_setrlimit	/* 75 */
	.long _sys_old_getrlimit
	.long _sys_getrusage
	.long _sys_gettimeofday
	.long _sys_settimeofday
	.long _sys_getgroups16	/* 80 */
	.long _sys_setgroups16
	.long _old_select
	.long _sys_symlink
	.long _sys_lstat
	.long _sys_readlink	/* 85 */
	.long _sys_uselib
	.long _sys_swapon
	.long _sys_reboot
	.long _old_readdir
	.long _old_mmap		/* 90 */
	.long _sys_munmap
	.long _sys_truncate
	.long _sys_ftruncate
	.long _sys_fchmod
	.long _sys_fchown16	/* 95 */
	.long _sys_getpriority
	.long _sys_setpriority
	.long _sys_ni_syscall	/* old profil syscall holder */
	.long _sys_statfs
	.long _sys_fstatfs	/* 100 */
	.long _sys_ioperm
	.long _sys_socketcall
	.long _sys_syslog
	.long _sys_setitimer
	.long _sys_getitimer	/* 105 */
	.long _sys_newstat
	.long _sys_newlstat
	.long _sys_newfstat
	.long _sys_uname
	.long _sys_iopl		/* 110 */
	.long _sys_vhangup
	.long _sys_ni_syscall	/* old "idle" system call */
	.long _sys_vm86old
	.long _sys_wait4
	.long _sys_swapoff	/* 115 */
	.long _sys_sysinfo
	.long _sys_ipc
	.long _sys_fsync
	.long _sys_sigreturn
	.long _sys_clone		/* 120 */
	.long _sys_setdomainname
	.long _sys_newuname
	.long _sys_modify_ldt
	.long _sys_adjtimex
	.long _sys_mprotect	/* 125 */
	.long _sys_sigprocmask
	.long _sys_ni_syscall	/* old "create_module" */
	.long _sys_init_module
	.long _sys_delete_module
	.long _sys_ni_syscall	/* 130:	old "get_kernel_syms" */
	.long _sys_quotactl
	.long _sys_getpgid
	.long _sys_fchdir
	.long _sys_bdflush
	.long _sys_sysfs		/* 135 */
	.long _sys_personality
	.long _sys_ni_syscall	/* reserved for afs_syscall */
	.long _sys_setfsuid16
	.long _sys_setfsgid16
	.long _sys_llseek	/* 140 */
	.long _sys_getdents
	.long _sys_select
	.long _sys_flock
	.long _sys_msync
	.long _sys_readv		/* 145 */
	.long _sys_writev
	.long _sys_getsid
	.long _sys_fdatasync
	.long _sys_sysctl
	.long _sys_mlock		/* 150 */
	.long _sys_munlock
	.long _sys_mlockall
	.long _sys_munlockall
	.long _sys_sched_setparam
	.long _sys_sched_getparam   /* 155 */
	.long _sys_sched_setscheduler
	.long _sys_sched_getscheduler
	.long _sys_sched_yield
	.long _sys_sched_get_priority_max
	.long _sys_sched_get_priority_min  /* 160 */
	.long _sys_sched_rr_get_interval
	.long _sys_nanosleep
	.long _sys_mremap
	.long _sys_setresuid16
	.long _sys_getresuid16	/* 165 */
	.long _sys_vm86
	.long _sys_ni_syscall	/* Old _sys_query_module */
	.long _sys_poll
	.long _sys_nfsservctl
	.long _sys_setresgid16	/* 170 */
	.long _sys_getresgid16
	.long _sys_prctl
	.long _sys_rt_sigreturn
	.long _sys_rt_sigaction
	.long _sys_rt_sigprocmask	/* 175 */
	.long _sys_rt_sigpending
	.long _sys_rt_sigtimedwait
	.long _sys_rt_sigqueueinfo
	.long _sys_rt_sigsuspend
	.long _sys_pread64	/* 180 */
	.long _sys_pwrite64
	.long _sys_chown16
	.long _sys_getcwd
	.long _sys_capget
	.long _sys_capset	/* 185 */
	.long _sys_sigaltstack
	.long _sys_sendfile
	.long _sys_ni_syscall	/* reserved for streams1 */
	.long _sys_ni_syscall	/* reserved for streams2 */
	.long _sys_vfork		/* 190 */
	.long _sys_getrlimit
	.long _sys_mmap2
	.long _sys_truncate64
	.long _sys_ftruncate64
	.long _sys_stat64	/* 195 */
	.long _sys_lstat64
	.long _sys_fstat64
	.long _sys_lchown
	.long _sys_getuid
	.long _sys_getgid	/* 200 */
	.long _sys_geteuid
	.long _sys_getegid
	.long _sys_setreuid
	.long _sys_setregid
	.long _sys_getgroups	/* 205 */
	.long _sys_setgroups
	.long _sys_fchown
	.long _sys_setresuid
	.long _sys_getresuid
	.long _sys_setresgid	/* 210 */
	.long _sys_getresgid
	.long _sys_chown
	.long _sys_setuid
	.long _sys_setgid
	.long _sys_setfsuid	/* 215 */
	.long _sys_setfsgid
	.long _sys_pivot_root
	.long _sys_mincore
	.long _sys_madvise
	.long _sys_getdents64	/* 220 */
	.long _sys_fcntl64
	.long _sys_ni_syscall	/* reserved for TUX */
	.long _sys_ni_syscall
	.long _sys_gettid
	.long _sys_readahead	/* 225 */
	.long _sys_setxattr
	.long _sys_lsetxattr
	.long _sys_fsetxattr
	.long _sys_getxattr
	.long _sys_lgetxattr	/* 230 */
	.long _sys_fgetxattr
	.long _sys_listxattr
	.long _sys_llistxattr
	.long _sys_flistxattr
	.long _sys_removexattr	/* 235 */
	.long _sys_lremovexattr
	.long _sys_fremovexattr
	.long _sys_tkill
	.long _sys_sendfile64
	.long _sys_futex		/* 240 */
	.long _sys_sched_setaffinity
	.long _sys_sched_getaffinity
	.long _sys_set_thread_area
	.long _sys_get_thread_area
	.long _sys_io_setup	/* 245 */
	.long _sys_io_destroy
	.long _sys_io_getevents
	.long _sys_io_submit
	.long _sys_io_cancel
	.long _sys_fadvise64	/* 250 */
	.long _sys_ni_syscall
	.long _sys_exit_group
	.long _sys_lookup_dcookie
	.long _sys_epoll_create
	.long _sys_epoll_ctl	/* 255 */
	.long _sys_epoll_wait
 	.long _sys_remap_file_pages
 	.long _sys_set_tid_address
 	.long _sys_timer_create
 	.long _sys_timer_settime		/* 260 */
 	.long _sys_timer_gettime
 	.long _sys_timer_getoverrun
 	.long _sys_timer_delete
 	.long _sys_clock_settime
 	.long _sys_clock_gettime		/* 265 */
 	.long _sys_clock_getres
 	.long _sys_clock_nanosleep
	.long _sys_statfs64
	.long _sys_fstatfs64	
	.long _sys_tgkill	/* 270 */
	.long _sys_utimes
 	.long _sys_fadvise64_64
	.long _sys_ni_syscall	/* _sys_vserver */
	.long _sys_ni_syscall	/* _sys_mbind */
	.long _sys_ni_syscall	/* 275 _sys_get_mempolicy */
	.long _sys_ni_syscall	/* _sys_set_mempolicy */
	.long _sys_mq_open
	.long _sys_mq_unlink
	.long _sys_mq_timedsend
	.long _sys_mq_timedreceive	/* 280 */
	.long _sys_mq_notify
	.long _sys_mq_getsetattr

syscall_table_size=(.-_sys_call_table)
nr_syscalls=((syscall_table_size)/4)

