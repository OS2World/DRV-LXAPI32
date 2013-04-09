/* $Id: oak_head.S,v 1.4 2006/02/16 23:07:20 smilcke Exp $ */

/*
 * oak_head.s
 * Autor:               Stefan Milcke
 * Erstellt am:         08.10.2004
 * Letzte Aenderung am: 05.06.2005
 *
*/


.text

#include <linux/config.h>
#include <linux/threads.h>
#include <linux/linkage.h>
#include <asm/segment.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/desc.h>
#include <asm/cache.h>
#include <asm/thread_info.h>
//#include <asm/asm_offsets.h>
#include <asm/setup.h>

/*
 * References to members of the new_cpu_data structure.
 */

#define CPU_PARAMS	_new_cpu_data
#define X86			CPU_PARAMS+0
#define X86_VENDOR		CPU_PARAMS+1
#define X86_MODEL		CPU_PARAMS+2
#define X86_MASK		CPU_PARAMS+3
#define X86_HARD_MATH	CPU_PARAMS+6
#define X86_CPUID		CPU_PARAMS+8
#define X86_CAPABILITY	CPU_PARAMS+12
#define X86_VENDOR_ID	CPU_PARAMS+36	/* offset dependent on NCAPINTS */

/*
 * This is how much memory *in addition to the memory covered up to
 * and including _end* we need mapped initially.  We need one bit for
 * each possible page, but only in low memory, which means
 * 2^32/4096/8 = 128K worst case (4G/4G split.)
 *
 * Modulo rounding, each megabyte assigned here requires a kilobyte of
 * memory, which is currently unreclaimed.
 *
 * This should be a multiple of a page.
 */
#define INIT_MAP_BEYOND_END	(128*1024)


ENTRY(startup_32)
ENTRY(_startup_32)
page_pde_offset = (__PAGE_OFFSET >> 20);
   jmp		checkCPUtype
	int		$3
	movl	$(_pg0 - __PAGE_OFFSET),%edi
	movl	$(_swapper_pg_dir - __PAGE_OFFSET),%edx
	movl	$0x007,%eax
10:
	leal	0x007(%edi),%ecx
	movl	%ecx,(%edx)
	movl	%ecx,page_pde_offset(%edx)
	addl	$4,%edx
	movl	$1024,%ecx
11:
	stosl
	addl	$0x1000,%eax
	loop	11b
	leal	(INIT_MAP_BEYOND_END+0x007)(%edi),%ebp
	jb		10b
	movl	%edi,(_init_pg_tables_end - __PAGE_OFFSET)

checkCPUtype:

	movl $-1,X86_CPUID		#  -1 for no CPUID initially

/* check if it is 486 or 386. */
/*
 * XXX - this does a lot of unnecessary setup.  Alignment checks don't
 * apply at our cpl of 0 and the stack ought to be aligned already, and
 * we don't need to preserve eflags.
 */

	movb $3,X86		# at least 386
	pushfl			# push EFLAGS
	popl %eax		# get EFLAGS
	movl %eax,%ecx		# save original EFLAGS
	xorl $0x240000,%eax	# flip AC and ID bits in EFLAGS
	pushl %eax		# copy to EFLAGS
	popfl			# set EFLAGS
	pushfl			# get new EFLAGS
	popl %eax		# put it in eax
	xorl %ecx,%eax		# change in flags
	pushl %ecx		# restore original EFLAGS
	popfl
	testl $0x40000,%eax	# check if AC bit changed
	je is386

	movb $4,X86		# at least 486
	testl $0x200000,%eax	# check if ID bit changed
	je is486

	/* get vendor info */
	xorl %eax,%eax			# call CPUID with 0 -> return vendor ID
	cpuid
	movl %eax,X86_CPUID		# save CPUID level
	movl %ebx,X86_VENDOR_ID		# lo 4 chars
	movl %edx,X86_VENDOR_ID+4	# next 4 chars
	movl %ecx,X86_VENDOR_ID+8	# last 4 chars

	orl %eax,%eax			# do we have processor info as well?
	je is486

	movl $1,%eax		# Use the CPUID instruction to get CPU type
	cpuid
	movb %al,%cl		# save reg for future use
	andb $0x0f,%ah		# mask processor family
	movb %ah,X86
	andb $0xf0,%al		# mask model
	shrb $4,%al
	movb %al,X86_MODEL
	andb $0x0f,%cl		# mask mask revision
	movb %cl,X86_MASK
	movl %edx,X86_CAPABILITY

is486:	movl $0x50022,%ecx	# set AM, WP, NE and MP
	jmp 2f

is386:	movl $2,%ecx		# set MP
2:	movl %cr0,%eax
	andl $0x80000011,%eax	# Save PG,PE,ET
	orl %ecx,%eax

	call check_x87
	incb ready
	jmp  _start_kernel

/*
 * We depend on ET to be correct. This checks for 287/387.
 */
check_x87:
	movb $0,X86_HARD_MATH
	clts
	fninit
	fstsw %ax
	cmpb $0,%al
	je 1f
	movl %cr0,%eax		/* no coprocessor: have to set bits */
	xorl $4,%eax		/* set EM */
	movl %eax,%cr0
	ret
	.align
1:	movb $1,X86_HARD_MATH
//	.byte 0xDB,0xE4		/* fsetpm for 287, ignored by 387 */
	ret
.data
ready:	.byte 0

	.globl	_swapper_pg_dir
	.align	4,0x90
	_swapper_pg_dir:
	.fill	1024,4,0

	.globl	_empty_zero_page
	.align	4,0x90
	_empty_zero_page:
	.fill	4096,1,0

/*
 * The Global Descriptor Table contains 28 quadwords, per-CPU.
 */
/*	.align PAGE_SIZE_asm */
	.globl _cpu_gdt_table
	.align 4,0x90
	_cpu_gdt_table:
	.quad 0x0000000000000000	/* NULL descriptor */
	.quad 0x0000000000000000	/* 0x0b reserved */
	.quad 0x0000000000000000	/* 0x13 reserved */
	.quad 0x0000000000000000	/* 0x1b reserved */
	.quad 0x0000000000000000	/* 0x20 unused */
	.quad 0x0000000000000000	/* 0x28 unused */
	.quad 0x0000000000000000	/* 0x33 TLS entry 1 */
	.quad 0x0000000000000000	/* 0x3b TLS entry 2 */
	.quad 0x0000000000000000	/* 0x43 TLS entry 3 */
	.quad 0x0000000000000000	/* 0x4b reserved */
	.quad 0x0000000000000000	/* 0x53 reserved */
	.quad 0x0000000000000000	/* 0x5b reserved */

	.quad 0x00cf9a000000ffff	/* 0x60 kernel 4GB code at 0x00000000 */
	.quad 0x00cf92000000ffff	/* 0x68 kernel 4GB data at 0x00000000 */
	.quad 0x00cffa000000ffff	/* 0x73 user 4GB code at 0x00000000 */
	.quad 0x00cff2000000ffff	/* 0x7b user 4GB data at 0x00000000 */

	.quad 0x0000000000000000	/* 0x80 TSS descriptor */
	.quad 0x0000000000000000	/* 0x88 LDT descriptor */

	/* Segments used for calling PnP BIOS */
	.quad 0x00c09a0000000000	/* 0x90 32-bit code */
	.quad 0x00809a0000000000	/* 0x98 16-bit code */
	.quad 0x0080920000000000	/* 0xa0 16-bit data */
	.quad 0x0080920000000000	/* 0xa8 16-bit data */
	.quad 0x0080920000000000	/* 0xb0 16-bit data */
	/*
	 * The APM segments have byte granularity and their bases
	 * and limits are set at run time.
	 */
	.quad 0x00409a0000000000	/* 0xb8 APM CS    code */
	.quad 0x00009a0000000000	/* 0xc0 APM CS 16 code (16 bit) */
	.quad 0x0040920000000000	/* 0xc8 APM DS    data */

	.quad 0x0000000000000000	/* 0xd0 - unused */
	.quad 0x0000000000000000	/* 0xd8 - unused */
	.quad 0x0000000000000000	/* 0xe0 - unused */
	.quad 0x0000000000000000	/* 0xe8 - unused */
	.quad 0x0000000000000000	/* 0xf0 - unused */
	.quad 0x0000000000000000	/* 0xf8 - GDT entry 31: double-fault TSS */

#ifdef CONFIG_SMP
	.fill (8-1)*32,8,0 /* other CPU's GDT */
#endif
