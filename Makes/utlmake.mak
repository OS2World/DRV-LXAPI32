# $Id: utlmake.mak,v 1.4 2005/05/12 20:54:46 smilcke Exp $

#==============================================================================
# Setup environment
#==============================================================================
.PHONY:	all clean
HELLOMSG	:= Building $(TARGET)

#==============================================================================
# Include Main Makefile
#==============================================================================
LXNOUSEREGPARMS	:= 1
PRE_INCLUDES	:=-I$(LXAPI32DEV)\lxlibc\include
PRE_CDEFINES	:=-DLXUTIL
include $(LXAPI32DEV)\makes\mainmake.mak

ifneq	"$(strip $(LXOMF))" ""
ifdef LXDEBUG
ICC	  :=ICC.EXE /Mc /Ss+ /Ti+ /C+
else
ICC	  :=ICC.EXE /Mc /Ss+ /G5 /Gd- /Ge+ /Gm+ /O+ /C+
endif
ICC	  +=/I$(LXAPI32DEV)\Include /I$(LXAPI32DEV)\lxlibc\include
endif

ifeq ($(strip $(ALLDEPS)),)
ALLDEPS	:=
endif
ALLDEPS	+= ShowHelloMsg makefile $(COMMONDEPENDENCIES)

ifneq ($(strip $(DIRS)),)
.PHONY:	$(DIRS)
ALLDEPS	+= $(DIRS)
endif

ifneq ($(strip $(OBJS)),)
MDBINRC	:=$(shell if not exist .\bin md .\bin 2> nul)
ALLDEPS	+= MakeBinDir $(DEPFILE) $(EXEFILE)

ifeq	"$(strip $(LXCLEAN))" ""
include $(DEPFILE)
endif

endif

LDOBJS		:=$(foreach O,$(OBJS),$(DRV_OBJ)\$(O))

ifneq	"$(strip $(LXOMF))" ""
LIBS	  :=$(GCC)\lib\libos2.lib $(LXAPI32DEV)\lxlibc\lib\lxlibc_s.$(LIB_EXT)
else
LIBS	  :=libos2 lxlibc_s
endif

ifneq	"$(strip $(LXOMF))" ""
LIBDEP		:= $(LIBS)
else
LIBDEP		:= $(LXAPI32DEV)\lxlibc\lib\lxlibc_s.a
endif

#===================================================================
#
#   Specific dependencies
#
#===================================================================
all: $(ALLDEPS)

ifndef OWNLINK
$(EXEFILE): $(OBJS)
	@echo --------------------- $@
	@if exist $(EXEFILE) del $(EXEFILE)
	@if exist $(TARGET).err del $(TARGET).err
	ld -r -o $(EXEFILE) $(LDOBJS)
	@$(SHELL) /C ChkErr.CMD $(TARGET).err 0 0
endif

$(DEPFILE): *.c
	@echo --------------------- Generating dependencies
	@$(CC) -M *.C > $(DEPFILE)
ifneq	"$(strip $(LXOMF))" ""
	@$(SHELL) /C CnvDep.cmd $(DEPFILE)
endif

ifdef DIRS
$(DIRS):
	@call $(MAKE) -C $@ $(MAKEOPTS) all
endif

clean: ShowHelloMsg MakeBinDir CleanCommonFiles
ifneq	"$(strip $(EXEFILE))" ""
	@if exist $(EXEFILE) del $(EXEFILE)
endif
ifneq	"$(strip $(SYMFILE))" ""
	@if exist $(SYMFILE) del $(SYMFILE)
endif
	@for %f in ($(DIRS)) do @call $(MAKE) -C %f clean
	-@rd bin
