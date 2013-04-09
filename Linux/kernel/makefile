# $Id: makefile,v 1.13 2005/07/25 20:04:22 smilcke Exp $

TARGET     = lxkernel
#LXFORCEO   := 1

#===================================================================
#   List of source files
#===================================================================
SRCFIRST	:=
SRCFILES	:=	lk_capability \
			lk_cpu \
			lk_exec_domain \
			lk_extable \
			lk_futex \
			lk_itimer \
			lk_kallsyms \
			lk_kmod \
			lk_kthread \
			lk_params \
			lk_pid \
			lk_posix-timers \
			lk_profile \
			lk_ptrace \
			lk_rcupdate \
			lk_resource \
			lk_setup \
			lk_signal \
			lk_softirq \
			lk_sys \
			lk_sysctl \
			lk_time \
			lk_user \
			lk_workqueue

SRCLAST		:=

EMXOMFAROPTS	:= -p32

include $(LXAPI32DEV)\makes\libmake.mak
