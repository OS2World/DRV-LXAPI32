# $Id: makefile,v 1.22 2005/07/17 00:03:47 smilcke Exp $

#===================================================================
# Main	makefile for LXAPI 32Bit device	driver (LXAPI32.SYS)
#===================================================================

#===================================================================
# Setup environment
#===================================================================
FDIRS	:= lib
DIRS	:= linux os2
LDIRS	:= dev32 Daemon
UDIRS	:= utils
MDIRS	:= modules
LCDIRS	:= lxlibc\src

CLDIRS	:= $(FDIRS) $(DIRS) $(LDIRS) $(LCDIRS) $(UDIRS) $(MDIRS)


.PHONY:	all msg $(DIRS) $(FDIRS) $(LDIRS) $(LCDIRS) $(UDIRS) $(MDIRS)

SHELL	=	$(COMSPEC)

MAKEOPTS	:= -wr

ifdef LXDEBUG
LXDEBUG		:= 1
MAKEOPTS	+= LXDEBUG=1
endif

all: msg .\makes\paths.mak .\include\defconfig.h $(FDIRS) $(DIRS) $(LDIRS) $(LCDIRS) $(UDIRS) $(MDIRS)
#all: msg .\makes\paths.mak .\include\defconfig.h $(MDIRS)


.\include\defconfig.h: .\include\defconfig tools\MkDefConf.CMD
	@$(SHELL) /C tools\MkDefConf.CMD .\include .\inc
.\inc\defcfg.inc: .\include\defconfig tools\MkDefConf.CMD
	@$(SHELL) /C tools\MkDefConf.CMD .\include .\inc

$(FDIRS):
	@call $(MAKE) -C $@ $(MAKEOPTS) LXOMF=1 LXDEFKERNEL=1 all

$(DIRS): $(FDIRS)
	@call $(MAKE) -C $@ $(MAKEOPTS) LXOMF=1 LXDEFKERNEL=1 all

$(LDIRS): $(DIRS)
	@call $(MAKE) -C $@ $(MAKEOPTS) LXOMF=1 LXDEFKERNEL=1 all

$(LCDIRS):
	@call $(MAKE) -C $@ $(MAKEOPTS) LXLIBC=1 all

$(UDIRS): $(LCDIRS)
	@call $(MAKE) -C $@ $(MAKEOPTS) LXUTIL=1 all

$(MDIRS): $(LCDIRS)
	@call $(MAKE) -C $@ $(MAKEOPTS) LXMODULE=1 LXDEFKERNEL=1 all

msg:
	@echo **********************************************************************
ifdef LXDEBUG
	@echo Building DEBUG Version
else
	@echo Building RELEASE Version
endif
	@echo **********************************************************************

.\makes\paths.mak:
	@call tools\config .\makes\paths.mak

clean:
	@echo Cleaning up directories ...
	@for %f in ($(CLDIRS)) do @call $(MAKE) -C %f clean
	@tools\config .\makes\paths.mak > nul
