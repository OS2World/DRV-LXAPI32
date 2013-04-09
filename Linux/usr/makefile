# $Id: makefile,v 1.6 2005/03/25 00:06:31 smilcke Exp $

TARGET		= lxusr
LXFORCEO   := 1

#===================================================================
#   List of source files
#===================================================================
SRCFIRST	:=
SRCFILES	:=	lu_initramfs \
				lu_initramfs_data
SRCLAST 	:=

BZ_FILE  :=bin\initramfs_data.cpio.gz
BZBFILE  :=bin\initramfs_data.cpio

ALLDEPS  := $(BZ_FILE)

include $(LXAPI32DEV)\makes\libmake.mak

#===================================================================
#   Specific dependencies
#===================================================================
$(BZ_FILE):
	gcc gen_init_cpio.c
	gen_init_cpio.exe > $(BZBFILE)
	del gen_init_cpio.exe
	gzip $(BZBFILE)
