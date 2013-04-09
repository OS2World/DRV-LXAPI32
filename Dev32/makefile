# $Id: makefile,v 1.49 2006/04/22 21:44:58 smilcke Exp $

#===================================================================
# Makefile for LXAPI32 driver
#===================================================================
TARGET     = LXAPI32
HELLOMSG	= Building $(TARGET)

#===================================================================
#   List of source files
#===================================================================
SRCFIRST	:=	lxheader
SRCFILES	:=	lxstartup \
			lxvarsa \
			lxstack \
			lxthreads \
			lxdevhlp \
			lxsecure \
			lxrmcall \
			lxdaemon \
			lxdispatch \
			lxidc \
			lxinit \
			lxioctl \
			lxmemlock \
			lxmemory \
			lxparam \
			lxrm \
			lxsegments \
			lxstrategy \
			lxszopt \
			lxuioctl \
			lxustrat \
			lxvars \
			lxwaiton
SRCLAST		:=	lxend

#==============================================================================
# Include Main Makefile
#==============================================================================
include $(LXAPI32DEV)\makes\mainmake.mak

LRFFILE  :=$(DRV_OBJ)\$(TARGET).lrf
MAPNAME  :=$(DRV_BIN)\$(TARGET).MAP
DEFFILE  :=$(DRV_OBJ)\$(TARGET).def
DEPFILE  :=$(DRV_OBJ)\$(TARGET).dep
GSYSTEM  :=$(DRV_OBJ)\lxsyms
GENSOPT  :=/OutFile:$(GSYSTEM)
ifdef CONFIG_OS2_KALLSYMS
GENSOPT  +=/GenKallsyms
endif

LIBS	  := $(wildcard $(DRV_LIB)/*.lib) $(DDK)\base\lib\vdh.lib

MDBINRC	:=$(shell if not exist .\bin md .\bin 2> nul)
include $(DEPFILE)

SEGCFGFILE	:=$(LXAPI32DEV)\inc\segments.cfg
SEGBEGINFILE	:=$(LXAPI32DEV)\inc\segbegin.inc

#===================================================================
#   Specific dependencies
#===================================================================
all: ShowHelloMsg MakeBinDir $(SEGBEGINFILE) $(LIBS) $(LRFFILE) \
     $(DRV_BIN)\$(TARGET).sys $(DRV_BIN)\$(TARGET).sym \
     makefile $(COMMONDEPENDENCIES)

$(SEGBEGINFILE): $(SEGCFGFILE)
	@$(SHELL) /C GSegments.cmd $(SEGCFGFILE) $(LXAPI32DEV)\inc

$(LRFFILE): makefile $(LIBS) $(COMMONDEPENDENCIES)
	@if exist $(LRFFILE) @del $(LRFFILE) > nul
ifeq "$(LNKNOFREEFORMAT)" "1"
	@for %f in ($(OBJFIRST)) do @echo ""$(DRV_OBJ)\%f'"+>> $(LRFFILE)
	@for %f in ($(OBJFILES)) do @echo ""$(DRV_OBJ)\%f'"+>> $(LRFFILE)
	@echo $(GSYSTEM).obj+ >> $(LRFFILE)
	@for %f in ($(DRV_LIB)\*.lib) do @echo ""%f'"+>> $(LRFFILE)
	@for %f in ($(OBJLAST)) do @echo ""$(DRV_OBJ)\%f'">> $(LRFFILE)

	@echo $(DRV_BIN)\$(TARGET).sys >> $(LRFFILE)
	@echo $(DRV_BIN)\$(TARGET).map >> $(LRFFILE)
	@echo $(DDK)\base\lib\vdh.lib >> $(LRFFILE)
	@echo $(DEFFILE) >> $(LRFFILE)
else
	@echo /DO >> $(LRFFILE)
	@echo /OUT:$(DRV_BIN)\$(TARGET).sys >> $(LRFFILE)
	@echo /MAP:$(DRV_BIN)\$(TARGET).map >> $(LRFFILE)
	@echo $(DEFFILE) >> $(LRFFILE)
	@for %f in ($(OBJFIRST)) do @echo ""$(DRV_OBJ)\%f'" >> $(LRFFILE)
	@for %f in ($(OBJFILES)) do @echo ""$(DRV_OBJ)\%f'" >> $(LRFFILE)
	@for %f in ($(DRV_LIB)\*.lib) do @echo ""%f'" >> $(LRFFILE)
	@echo $(DDK)\base\lib\vdh.lib >> $(LRFFILE)
	@echo $(GSYSTEM).obj >> $(LRFFILE)
	@for %f in ($(OBJLAST)) do @echo ""$(DRV_OBJ)\%f'" >> $(LRFFILE)
endif

$(DRV_BIN)\$(TARGET).sys: $(OBJSALL) $(LIBS) $(LRFFILE) $(SEGBEGINFILE)
	@echo -------------------- $(TARGET).sys
	@echo PHYSICAL DEVICE > $(DEFFILE)
	@echo DESCRIPTION '@#$(_VENDOR):$(_VERSION).$(BUILD_LEVEL)#@ Linux API Support Driver' >> $(DEFFILE)
	@echo SEGMENTS >> $(DEFFILE)
	@echo DATA16 CLASS "'DATA16'" >> $(DEFFILE)
	@echo CODE16 CLASS "'CODE'" >> $(DEFFILE)
	@echo TEXT32 CLASS "'CODE'" >> $(DEFFILE)
	@echo DATA32 CLASS "'DATA'" >> $(DEFFILE)
	@echo BSS32 CLASS "'BSS'" >> $(DEFFILE)
	@echo _init_text CLASS "'CODE'" >> $(DEFFILE)
	@echo _sched_text CLASS "'CODE'" >> $(DEFFILE)
	@echo _exit_text CLASS "'CODE'" >> $(DEFFILE)
	@echo _init_data CLASS "'_init_data'" READWRITE >> $(DEFFILE)
	@echo _init_setup CLASS "'_init_setup'" READWRITE >> $(DEFFILE)
	@echo _initcall1_init CLASS "'_initcall1_init'" READWRITE >> $(DEFFILE)
	@echo _initcall2_init CLASS "'_initcall2_init'" READWRITE >> $(DEFFILE)
	@echo _initcall3_init CLASS "'_initcall3_init'" READWRITE >> $(DEFFILE)
	@echo _initcall4_init CLASS "'_initcall4_init'" READWRITE >> $(DEFFILE)
	@echo _initcall5_init CLASS "'_initcall5_init'" READWRITE >> $(DEFFILE)
	@echo _initcall6_init CLASS "'_initcall6_init'" READWRITE >> $(DEFFILE)
	@echo _initcall7_init CLASS "'_initcall7_init'" READWRITE >> $(DEFFILE)
	@echo _exitcall_exit CLASS "'_exitcall_exit'" READWRITE >> $(DEFFILE)
#	@echo ELFBSS CLASS "'BSS'" >> $(DEFFILE)

ifneq "$(strip $(CNV_OBJS))" ""
	@for %f in ($(CNV_OBJS)) do $(SHELL) /C ChgSeg.cmd $(DRV_OBJ)\%f TEXT32 TEXT99
endif

	@$(SHELL) /C GCSyms.cmd /GenEmpty $(GENSOPT)
	$(GCCAOUT) -c $(GSYSTEM).S -o$(GSYSTEM).o -Wa,-ahls=$(GSYSTEM).lst 2> $(GSYSTEM).err
	$(OSTOOBJ) $(GSYSTEM).o 2>> $(GSYSTEM).err
	@if exist $(GSYSTEM).err del $(GSYSTEM).err > nul

	@echo Linking Step 1 ...
	$(LNKTOOL) @$(LRFFILE) > $(DRV_BIN)\$(TARGET).err
	@if exist $(DRV_BIN)\$(TARGET).sys @del $(DRV_BIN)\$(TARGET).sys
	@$(SHELL) /C ChkErr.cmd $(DRV_BIN)\$(TARGET).err 1 0

	@echo Generating symbols ...
	@$(SHELL) /C GCSyms.cmd /Mapfile:$(DRV_BIN)\$(TARGET).map $(GENSOPT)
	$(GCCAOUT) -c $(GSYSTEM).S -o$(GSYSTEM).o -Wa,-ahls=$(GSYSTEM).lst 2> $(GSYSTEM).err
	$(OSTOOBJ) $(GSYSTEM).o 2>> $(GSYSTEM).err
	@if exist $(GSYSTEM).err del $(GSYSTEM).err > nul

	@echo Linking Step 2 ...
	$(LNKTOOL) @$(LRFFILE) > $(DRV_BIN)\$(TARGET).err
	@$(SHELL) /C ChkErr.cmd $(DRV_BIN)\$(TARGET).err 1 0

$(DRV_BIN)\$(TARGET).sym: $(DRV_BIN)\$(TARGET).SYS
	@echo --------------------- $(TARGET).sym
	mapsym $(MAPNAME)
	@copy $(TARGET).SYM $(DRV_BIN)\$(TARGET).sym > nul
	@if exist $(TARGET).SYM del $(TARGET).SYM > nul

$(DEPFILE): *.c
	@echo --------------------- Generating dependencies
	@$(CC) -M *.C > $(DEPFILE)
	@$(SHELL) /C CnvDep.cmd $(DEPFILE)

clean: ShowHelloMsg MakeBinDir CleanCommonFiles
	@if exist $(DRV_BIN)\$(TARGET).sys del $(DRV_BIN)\$(TARGET).sys
	@if exist $(DRV_BIN)\$(TARGET).sym del $(DRV_BIN)\$(TARGET).sym
	@if exist $(DEFFILE) del $(DEFFILE)
	@if exist $(DRV_BIN)\$(GSYSTEM).c del $(DRV_BIN)\$(GSYSTEM).c
