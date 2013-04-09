# $Id: makefile,v 1.8 2005/03/15 23:38:11 smilcke Exp $

TARGET     = oxarchkernel

#==============================================================================
# Setup subdirectories
#==============================================================================
DIRS	:= cpu

#===================================================================
#   List of source files
#===================================================================
SRCFIRST	:=
SRCFILES	:=	oak_apic \
			oak_entry \
			oak_head \
			oak_i8259 \
			oak_init_task \
			oak_io_apic \
			oak_irq \
			oak_process \
			oak_setup \
			oak_smpboot \
			oak_time \
			oak_traps \
			oak_vm86
SRCLAST		:=

include $(LXAPI32DEV)\makes\libmake.mak
