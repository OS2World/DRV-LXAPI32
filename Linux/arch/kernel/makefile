# $Id: makefile,v 1.12 2005/03/25 00:06:24 smilcke Exp $

TARGET     = lxarchkernel
LXFORCEO   := 1

#==============================================================================
# Setup subdirectories
#==============================================================================
DIRS	:= cpu timers

#===================================================================
#   List of source files
#===================================================================
SRCFIRST	:=
SRCFILES	:=lak_cpuid \
			lak_doublefault \
			lak_ldt \
			lak_module \
			lak_nmi \
			lak_pci-dma \
			lak_std_resources \
			lak_smpboot \
			lak_smp
SRCLAST		:=

include $(LXAPI32DEV)\makes\libmake.mak
