# $Id: makefile,v 1.10 2005/12/26 23:42:06 smilcke Exp $

#===================================================================
# Makefile for LXAPI32 shell
#===================================================================
TARGET     = init
HELLOMSG	= Building $(TARGET)
#OWNCTOOBJ	= 1
LXOMF		= 1
LXNOUSEREGPARMS	:= 1
USEELF:=0

#==============================================================================
# Setup environment
#==============================================================================
.PHONY:	all

#===================================================================
#   List of source files
#===================================================================
SRCFIRST	:=
SRCFILES	:=init
SRCLAST	:=

#==============================================================================
# Include Main Makefile
#==============================================================================
include $(LXAPI32DEV)\makes\utlmake.mak

ifneq	"$(strip $(LXOMF))" ""
LIB      :=$(OLD_LIB);$(LXAPI32DEV)\lxlibc\lib
INCLUDE  :=$(OLD_INCLUDE)
else
#LIB	:=$(LXAPI32DEV)\lxlibc\lib
#LIBRARY_PATH	:=$(LXAPI32DEV)\lxlibc\lib
endif

ifneq	"$(strip $(LXOMF))" ""
LRFFILE  :=$(DRV_OBJ)\$(TARGET).lrf
else
LRFFILE  :=
endif
MAPNAME  :=$(DRV_OBJ)\$(TARGET).MAP
DEFFILE  :=$(DRV_OBJ)\$(TARGET).def
DEPFILE  :=$(DRV_OBJ)\$(TARGET).dep
EXEFILE  :=$(LX_ROOT)\sbin\$(TARGET).exe
SYMFILE  :=$(LX_ROOT)\sbin\$(TARGET).sym

include $(DEPFILE)

#===================================================================
#   Specific dependencies
#===================================================================
all: ShowHelloMsg MakeBinDir $(LRFFILE) $(DEFFILE) $(EXEFILE) $(LIBDEP) makefile $(COMMONDEPENDENCIES)

$(LRFFILE): makefile $(COMMONDEPENDENCIES)
	@if exist $(LRFFILE) @del $(LRFFILE)
	@for %f in ($(OBJSALL)) do @echo ""$(DRV_OBJ)\%f'" >> $(LRFFILE)
	@echo /OUT:$(EXEFILE) >> $(LRFFILE)
	@echo /MAP:$(MAPNAME) >> $(LRFFILE)
	@echo $(DEFFILE) >> $(LRFFILE)
	@echo $(CRT0) $(LIBS) cppom30.lib >> $(LRFFILE)

$(EXEFILE): $(OBJSALL) $(LRFFILE) $(LIBDEP) $(DEFFILE)
	@echo -------------------- $(EXEFILE)
ifdef LXDEBUG
	ILINK /NOLOGO /DEBUG @$(LRFFILE)
else
	ILINK /NOLOGO /PACKCODE /PACKDATA @$(LRFFILE)
endif
	$(SHELL) /C ChkErr.cmd $(DRV_BIN)\$(TARGET).err 1 0

$(DEFFILE): makefile
	@echo NAME INIT WINDOWCOMPAT > $(DEFFILE)
	@echo DESCRIPTION '@#$(_VENDOR):$(_VERSION).$(BUILD_LEVEL)#@ Linux API INIT Process' >> $(DEFFILE)
	@echo STACKSIZE 32767 >> $(DEFFILE)
	@echo PROTMODE >> $(DEFFILE)
	@echo SEGMENTS >> $(DEFFILE)
	@echo CODE32 PRELOAD >> $(DEFFILE)
	@echo DATA32 PRELOAD >> $(DEFFILE)
