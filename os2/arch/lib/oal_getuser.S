/* $Id: oal_getuser.S,v 1.2 2006/02/28 19:46:52 smilcke Exp $ */

/*
 * oal_getuser.S
 * Autor:               Stefan Milcke
 * Erstellt am:         13.02.2005
 * Letzte Aenderung am: 27.02.2006
 *
*/

/*
 * __get_user functions.
 *
 * (C) Copyright 1998 Linus Torvalds
 *
 * These functions have a non-standard call interface
 * to make them more efficient, especially as they
 * return an error value in addition to the "real"
 * return value.
 */
#include <asm/thread_info.h>

/*
 * __get_user_X
 *
 * Inputs:	%eax contains the address
 *
 * Outputs:	%eax is error code (0 or -EFAULT)
 *		%edx contains zero-extended value
 *
 * These functions should not modify any other registers,
 * as they get called from within inline assembly.
 */

.text
.align 4
#ifdef OS2OLDGCC
.globl ___get_user_1
___get_user_1:
#else
.globl __get_user_1
__get_user_1:
#endif
	GET_THREAD_INFO(%edx)
	cmpl TI_ADDR_LIMIT(%edx),%eax
	jae bad_get_user
1:	movzbl (%eax),%edx
	xorl %eax,%eax
	ret

.align 4
#ifdef OS2OLDGCC
.globl ___get_user_2
___get_user_2:
#else
.globl __get_user_2
__get_user_2:
#endif
	addl $1,%eax
	jc bad_get_user
	GET_THREAD_INFO(%edx)
	cmpl TI_ADDR_LIMIT(%edx),%eax
	jae bad_get_user
2:	movzwl -1(%eax),%edx
	xorl %eax,%eax
	ret

.align 4
#ifdef OS2OLDGCC
.globl ___get_user_4
___get_user_4:
#else
.globl __get_user_4
__get_user_4:
#endif
	addl $3,%eax
	jc bad_get_user
	GET_THREAD_INFO(%edx)
	cmpl TI_ADDR_LIMIT(%edx),%eax
	jae bad_get_user
3:	movl -3(%eax),%edx
	xorl %eax,%eax
	ret

bad_get_user:
	xorl %edx,%edx
	movl $-14,%eax
	ret

#ifndef TARGET_OS2
.section __ex_table,"a"
	.long 1b,bad_get_user
	.long 2b,bad_get_user
	.long 3b,bad_get_user
.previous
#endif
