
# $Id: makefile,v 1.3 2005/12/26 23:42:07 smilcke Exp $

#===================================================================
# Makefile for LXAPI32 shell
#===================================================================
TARGET     = lxsh
HELLOMSG	= Building $(TARGET)
OWNCTOOBJ	= 1
USEELF:=0

#==============================================================================
# Setup environment
#==============================================================================
.PHONY:	all

#==============================================================================
# Include Main Makefile
#==============================================================================
include $(LXAPI32DEV)\makes\mainmake.mak

#===================================================================
#   List of source files
#===================================================================
OBJSFIRST:=
OBJS     :=lxsh.obj
OBJSLAST :=

OBJS	  :=$(OBJSFIRST) $(OBJS) $(OBJSLAST)


LIB      :=$(OLD_LIB)
INCLUDE  :=$(OLD_INCLUDE)

LRFFILE  :=$(DRV_OBJ)\$(TARGET).lrf
MAPNAME  :=$(DRV_OBJ)\$(TARGET).MAP
DEFFILE  :=$(TARGET).def
DEPFILE  :=$(DRV_OBJ)\$(TARGET).dep
LIBS	  :=OS2386.LIB

ifdef LXDEBUG
ICC	  :=ICC.EXE /Ss+ /C+
else
ICC	  :=ICC.EXE /Ss+ /G5 /Gd- /Ge+ /Gm+ /O+ /C+
endif
ICC	  +=/I$(LXAPI32DEV)\Include /I$(LXAPI32DEV)\lxlibc\include

include $(DEPFILE)

%.obj: %.c $(COMMONDEPENDENCIES)
	@echo --------------------- $@
	@if exist *.err @del *.err
	$(ICC) /Fo.\$(DRV_OBJ)\$*.obj $<
	@if exist *.err @del *.err

#===================================================================
#   Specific dependencies
#===================================================================
all: ShowHelloMsg MakeBinDir $(LRFFILE) $(LX_ROOT)\sbin\$(TARGET).exe makefile $(COMMONDEPENDENCIES)

$(LRFFILE): makefile $(COMMONDEPENDENCIES)
	@if exist $(LRFFILE) @del $(LRFFILE)
	@for %f in ($(OBJS)) do @echo ""$(DRV_OBJ)\%f'" >> $(LRFFILE)
	@echo /OUT:$(LX_ROOT)\sbin\$(TARGET).exe >> $(LRFFILE)
	@echo /MAP:$(MAPNAME) >> $(LRFFILE)
	@echo $(DEFFILE) >> $(LRFFILE)
	@echo $(LIBS) >> $(LRFFILE)

$(LX_ROOT)\sbin\$(TARGET).exe: $(OBJS) $(OBJSLAST) $(LRFFILE)
	@echo -------------------- $(LX_ROOT)\sbin\$(TARGET).exe
	@$(BLDMGR) /FILE:$(LRFFILE) /TEXT:@#$(_VENDOR):$(_VERSION). /ACTION:SET:$(BUILD_LEVEL)
ifdef LXDEBUG
	ILINK /NOLOGO /DEBUG @$(LRFFILE)
else
	ILINK /NOLOGO /PACKCODE /PACKDATA @$(LRFFILE)
endif
	$(SHELL) /C ChkErr.cmd $(DRV_BIN)\$(TARGET).err 1 0

$(DEPFILE): *.c
	@echo --------------------- Generating dependencies
	@$(CC) -M *.C > $(DEPFILE)
	@$(SHELL) /C CnvDep.cmd $(DEPFILE)

clean: ShowHelloMsg MakeBinDir CleanCommonFiles
	@if exist $(LX_ROOT)\sbin\$(TARGET).exe del $(LX_ROOT)\sbin\$(TARGET).exe
	@if exist $(LX_ROOT)\sbin\$(TARGET).sym del $(LX_ROOT)\sbin\$(TARGET).sym
