# $Id: makefile,v 1.12 2005/06/13 22:16:33 smilcke Exp $

#===================================================================
# Makefile for LXAPI32 daemon
#===================================================================
TARGET     = lxapid
HELLOMSG	= Building $(TARGET)
OWNCTOOBJ	= 1

#==============================================================================
# Setup environment
#==============================================================================
.PHONY:	all

#===================================================================
#   List of source files
#===================================================================
SRCFIRST	:=
SRCFILES	:=	lxapid_cmd \
				lxapid \
				lxerrmap \
			    eah
SRCLAST		:=

#==============================================================================
# Include Main Makefile
#==============================================================================
include ..\makes\mainmake.mak


LIB      :=$(OLD_LIB)
INCLUDE  :=$(OLD_INCLUDE)

LRFFILE  :=$(DRV_OBJ)\$(TARGET).lrf
MAPNAME  :=$(DRV_BIN)\$(TARGET).MAP
DEFFILE  :=$(TARGET).def
DEPFILE  :=$(DRV_OBJ)\$(TARGET).dep
LIBS	 :=OS2386.LIB CPPOM30.LIB

ifdef LXDEBUG
#ICC	  :=ICC.EXE /Ss+ /Ti /C+
ICC	  :=ICC.EXE /Ss+ /G5 /Gd- /Ge+ /Gm+ /O+ /Oc+ /C+
else
ICC	  :=ICC.EXE /Ss+ /G5 /Gd- /Ge+ /Gm+ /O+ /Oc+ /C+
endif
ICC	  +=/I$(LXAPI32DEV)\Include

include $(DEPFILE)

%.obj: %.c $(COMMONDEPENDENCIES)
	@echo --------------------- $@
	@if exist *.err @del *.err
	$(ICC) /Fo.\$(DRV_OBJ)\$*.obj $<
	@if exist *.err @del *.err

#===================================================================
#   Specific dependencies
#===================================================================
all: ShowHelloMsg MakeBinDir $(LRFFILE) $(DRV_BIN)\$(TARGET).exe makefile $(COMMONDEPENDENCIES)

$(LRFFILE): makefile $(COMMONDEPENDENCIES)
	@if exist $(LRFFILE) @del $(LRFFILE)
	@for %f in ($(OBJSALL)) do @echo ""$(DRV_OBJ)\%f'" >> $(LRFFILE)
	@echo /OUT:$(DRV_BIN)\$(TARGET).exe >> $(LRFFILE)
	@echo /MAP:$(MAPNAME) >> $(LRFFILE)
	@echo $(DEFFILE) >> $(LRFFILE)
	@echo $(LIBS) >> $(LRFFILE)

$(DRV_BIN)\$(TARGET).exe: $(OBJSALL) $(LRFFILE)
	@echo -------------------- $(TARGET).exe
	@$(BLDMGR) /FILE:$(LRFFILE) /TEXT:@#$(_VENDOR):$(_VERSION). /ACTION:SET:$(BUILD_LEVEL)
ifdef LXDEBUG
#	ILINK /NOLOGO /NOD /DEBUG @$(LRFFILE)
	ILINK /NOLOGO /NOD /PACKCODE /PACKDATA /OPTFUNC @$(LRFFILE)
else
	ILINK /NOLOGO /NOD /PACKCODE /PACKDATA /OPTFUNC @$(LRFFILE)
endif
	$(SHELL) /C ChkErr.cmd $(DRV_BIN)\$(TARGET).err 1 0

$(DEPFILE): MakeBinDir *.c
	@echo --------------------- Generating dependencies
	@$(CC) -M *.C > $(DEPFILE)
	@$(SHELL) /C CnvDep.cmd $(DEPFILE)

clean: ShowHelloMsg MakeBinDir CleanCommonFiles
	@if exist $(DRV_BIN)\$(TARGET).exe del $(DRV_BIN)\$(TARGET).exe
	@if exist $(DRV_BIN)\$(TARGET).sym del $(DRV_BIN)\$(TARGET).sym
