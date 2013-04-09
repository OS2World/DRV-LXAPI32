# $Id: makefile,v 1.8 2005/04/26 22:13:23 smilcke Exp $

TARGET     = oxmm

#===================================================================
#   List of source files
#===================================================================
include $(LXAPI32DEV)\include\defconfig
mmu-y		:=	om_nommu
mmu-$(CONFIG_MMU)	:= om_memory om_rmap om_vmalloc

SRCFIRST	:=
SRCFILES	:=	om_bootmem \
			om_filemap \
			om_mmap \
			om_page_alloc \
			om_page-writeback \
			om_pdflush \
			om_readahead \
			om_slab \
			om_swap \
			om_truncate \
			om_vmscan \
			$(mmu-y)
SRCLAST		:=

include $(LXAPI32DEV)\makes\libmake.mak
