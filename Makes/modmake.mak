# $Id: modmake.mak,v 1.4 2005/12/06 22:26:34 smilcke Exp $

#==============================================================================
# Setup environment
#==============================================================================
.PHONY:	all clean
HELLOMSG	:= Building $(TARGET)

#==============================================================================
# Include Main Makefile
#==============================================================================
OWNCTOOBJ	:= 1
OWNSTOOBJ	:= 1
OWNMAKELIB	:= 1
include $(LXAPI32DEV)\makes\mainmake.mak

ifeq ($(strip $(ALLDEPS)),)
ALLDEPS	:=
endif
ALLDEPS	+= ShowHelloMsg makefile $(COMMONDEPENDENCIES)

ifneq ($(strip $(DIRS)),)
.PHONY:	$(DIRS)
ALLDEPS	+= $(DIRS)
endif

MODFILE	:=	$(SBINPATH)\$(TARGET)

ifneq ($(strip $(SRCFILES)),)
MDBINRC	:=$(shell if not exist .\bin md .\bin 2> nul)
ALLDEPS	+= MakeBinDir $(DEPFILE) $(MODFILE)

ifeq	"$(strip $(LXCLEAN))" ""
include $(DEPFILE)
endif

endif

MODSYMSTEM	:=$(DRV_OBJ)\$(TARGET)_modsyms
MODSYMS	:=$(MODSYMSTEM).S
MODSYMO	:=$(MODSYMSTEM).o

#===================================================================
#   Specific dependencies
#===================================================================
all: $(ALLDEPS)

$(MODFILE): $(OBJSALL) $(MODSYMO)
	@echo --------------------- $@
	@if exist *.err @del *.err
	@$(ASELF) $(MODSYMS) -o $(MODSYMO) 2>> $(TARGET).err
	$(LDELF) -r -o $(MODFILE) $(addprefix $(DRV_OBJ)\,$(OBJSALL)) $(MODSYMO)
	@if exist *.err @del *.err

$(MODSYMO): $(MODSYMS)
	@if exist *.err @del *.err
	@$(ASELF) $(MODSYMS) -o $(MODSYMO) 2>> $(NODSYMSTEM).err
	@if exist *.err @del *.err

%.$(OBJ_EXT): %.c $(COMMONDEPENDENCIES)
	@echo --------------------- $@
	@if exist *.err @del *.err
	$(CC) -elf -S $< -o $(DRV_OBJ)\$*.S -DKBUILD_BASENAME=$(subst $(comma),_,$(subst -,_,$(*F))) 2> $*.err
	@$(ASELF) $(DRV_OBJ)\$*.S -o $(DRV_OBJ)\$*.o 2>> $*.err
	@if exist *.err @del *.err

%.$(OBJ_EXT): %.S $(COMMONDEPENDENCIES)
	@echo --------------------- $@
	@if exist *.err @del *.err


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

clean: ShowHelloMsg MakeBinDir CleanCommonFiles
	@for %f in ($(DIRS)) do @call $(MAKE) -C %f clean
	-@rd bin
