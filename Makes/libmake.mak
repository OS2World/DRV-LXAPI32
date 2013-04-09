# $Id: libmake.mak,v 1.7 2005/12/13 22:24:48 smilcke Exp $

#==============================================================================
# Setup environment
#==============================================================================
.PHONY:	all clean
HELLOMSG	:= Building $(TARGET)

#==============================================================================
# Include Main Makefile
#==============================================================================
include $(LXAPI32DEV)\makes\mainmake.mak

ifeq ($(strip $(ALLDEPS)),)
ALLDEPS	:=
endif
ALLDEPS	+= ShowHelloMsg makefile $(COMMONDEPENDENCIES)

ifneq ($(strip $(DIRS)),)
.PHONY:	$(DIRS)
ALLDEPS	+= $(DIRS)
endif

ifneq ($(strip $(SRCFILES)),)
MDBINRC	:=$(shell if not exist .\bin md .\bin 2> nul)
ALLDEPS	+= MakeBinDir $(DEPFILE) $(LIBFILE)

ifeq	"$(strip $(LXCLEAN))" ""
include $(DEPFILE)
endif

endif

#===================================================================
#   Specific dependencies
#===================================================================
all: $(ALLDEPS)

ifeq "$(strip $(OWNMAKELIB))" ""
LIBBAK		:=	$(patsubst %.$(LIB_EXT),%.bak,$(LIBFILE))
$(LIBFILE): $(OBJSALL)
	@echo --------------------- $@
	@if exist $(LIBFILE) del $(LIBFILE)
	@if exist $(TARGET).err del $(TARGET).err
ifeq "$(strip $(LXOMF))" ""
	@ar -r $(LIBFILE) $(addprefix $(DRV_OBJ)\,$(OBJSALL)) 2>> $(TARGET).err
else
	@emxomfar $(EMXOMFAROPTS) r $(LIBFILE) $(addprefix $(DRV_OBJ)\,$(OBJSALL)) 2>> $(TARGET).err
#	@for %f in ($(OBJSALL)) do @emxomfar $(EMXOMFAROPTS) r $(LIBFILE) $(DRV_OBJ)\%f 2>> $(TARGET).err
endif
	@if exist $(LIBBAK) @del $(LIBBAK)
	@$(SHELL) /C ChkErr.CMD $(TARGET).err 0 0
endif

ifeq "$(strip $(OWNMAKEDEP))" ""
$(DEPFILE): $(strip $(wildcard *.c) $(wildcard *.S))
	@echo --------------------- Generating dependencies
ifneq "$(strip $(wildcard *.c) $(wildcard *.S))" ""
	@$(CC) -M $(strip $(wildcard *.c) $(wildcard *.S)) > $(DEPFILE)
endif
ifneq "$(strip $(LXOMF))" ""
	@$(SHELL) /C CnvDep.cmd $(DEPFILE)
endif
	@echo $(ALLDEPS)
endif

ifdef DIRS
$(DIRS):
	@call $(MAKE) -C $@ $(MAKEOPTS) all
endif

LI1	:=	$(patsubst %.a,%.lib,$(LIBFILE))
LI2	:=	$(patsubst %.lib,%.a,$(LIBFILE))
clean: ShowHelloMsg MakeBinDir CleanCommonFiles
	@if exist $(LI1) del $(LI1)
	@if exist $(LI2) del $(LI2)
	@for %f in ($(DIRS)) do @call $(MAKE) -C %f clean
	-@rd bin
