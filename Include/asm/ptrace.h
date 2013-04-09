#ifndef _I386_PTRACE_H
#define _I386_PTRACE_H

#define EBX 0
#define ECX 1
#define EDX 2
#define ESI 3
#define EDI 4
#define EBP 5
#define EAX 6
#define DS 7
#define ES 8
#define FS 9
#define GS 10
#define ORIG_EAX 11
#define EIP 12
#define CS  13
#define EFL 14
#define UESP 15
#define SS   16
#define FRAME_SIZE 17

/* this struct defines the way the registers are stored on the 
   stack during a system call. */

struct pt_regs {
	long ebx;	// 0
	long ecx;	// 4
	long edx;	// 8
	long esi;	// 12
	long edi;	// 16
	long ebp;	// 20
	long eax;	// 24
	int  xds;	// 28
	int  xes;	// 32
	long orig_eax; 	// 36
	long eip;	// 40
	int  xcs;	// 44
	long eflags;	// 48
	long esp;	// 52
	int  xss;	// 56
};

/* Arbitrarily choose the same ptrace numbers as used by the Sparc code. */
#define PTRACE_GETREGS            12
#define PTRACE_SETREGS            13
#define PTRACE_GETFPREGS          14
#define PTRACE_SETFPREGS          15
#define PTRACE_GETFPXREGS         18
#define PTRACE_SETFPXREGS         19

#define PTRACE_OLDSETOPTIONS         21

#define PTRACE_GET_THREAD_AREA    25
#define PTRACE_SET_THREAD_AREA    26

#ifdef __KERNEL__
#define user_mode(regs) ((VM_MASK & (regs)->eflags) || (3 & (regs)->xcs))
#define instruction_pointer(regs) ((regs)->eip)
#endif

#endif
