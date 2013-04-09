/* $Id: lxlcalloca.S,v 1.2 2005/03/30 00:19:15 smilcke Exp $ */

/*
 * lxlcalloca.S
 * Autor:               Stefan Milcke
 * Erstellt am:         30.03.2005
 * Letzte Aenderung am: 30.03.2005
 *
*/

#define	ALIGN	.align	2, 0x90

#define LABEL0(name)    _##name
#define LABEL(name)     LABEL0(name)

#if defined (__EPILOGUE__)
#define EPILOGUE_NO_RET(name) LABEL(__POST$##name):
#else
#define EPILOGUE_NO_RET(name)
#endif

		.text
		.globl	__alloca
		ALIGN
__alloca:
		pushl	%ecx				/* save work registers */
		movl	%esp,%ecx			/* keep a pointer to stack frame */
		negl	%eax
		leal	4+4(%esp,%eax),%esp	/* adjust stack pointer */
		leal	4+4(%ecx),%eax
		ALIGN
L1:	subl	$0x1000,%eax		/* step down */
		cmpl	%esp,%eax
		jb		L2
		testb	%al,(%eax)			/* probe stack */
		jmp		L1
L2:	movl	4(%ecx),%eax		/* return address */
		movl	(%ecx),%ecx			/* pop ECX */
		EPILOGUE_NO_RET(_alloca)
		jmp		*%eax				/* return */
