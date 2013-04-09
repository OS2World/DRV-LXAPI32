/* $Id: lxstack.h,v 1.5 2005/03/22 22:34:32 smilcke Exp $ */

/*
 * lxstack.h
 * Autor:               Stefan Milcke
 * Erstellt am:         16.01.2005
 * Letzte Aenderung am: 14.03.2005
 *
*/

#ifndef LXSTACK_H_INCLUDED
#define LXSTACK_H_INCLUDED

#define PDDSTACK_FLAG_FREE    (0)
#define PDDSTACK_FLAG_USED    (1)
#define PDDSTACK_FLAG_NOTASK  (0)
#define PDDSTACK_FLAG_ISTASK  (2)

struct PDDStack
{
 struct PDDStack* next;
 unsigned long base;
 unsigned long flags;
 unsigned short pid;
 unsigned short tid;
};

#define UTLSTACK_FLAG_FREE       (0)
#define UTLSTACK_FLAG_USED       (1)
#define UTLSTACK_FLAG_CAPTURED   (4)
#define UTLSTACK_FLAG_CAPTUREACK (8)
#define UTLSTACK_FLAG_FREEME     (16)

struct UTLStack
{
 struct UTLStack* next;
 unsigned long base;
 unsigned long flags;
 unsigned short pid;
 unsigned short tid;
};

extern struct PDDStack *lx_StackRoot;
extern struct PDDStack *lx_IrqStackRoot;
extern struct UTLStack *lx_UtlStackRoot;
extern unsigned long* lx_privateStack;

struct thread_info;
extern int LX_set_ti_for_pidtid(struct thread_info* ti
				,unsigned short pid
				,unsigned short tid);
extern int LX_unset_ti_for_pidtid(struct thread_info* ti
				  ,unsigned short pid
				  ,unsigned short tid);



#endif //LXSTACK_H_INCLUDED
