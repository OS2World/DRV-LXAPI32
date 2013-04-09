# $Id: mainmake.mak,v 1.66 2006/04/23 21:01:46 smilcke Exp $
.SUFFIXES:
.SUFFIXES:	.c .asm .lib .def .obj .lrf .i .s .dep
SHELL	=	$(COMSPEC)

ifndef USEELF
USEELF	=	1
endif

ifeq "$(USEELF)" "1"
OLDGCC	:=	1
endif

#USELINK386	=	1

#==============================================================================
# Standard vars
#==============================================================================
comma   := ,
empty   :=
space   := $(empty) $(empty)

#==============================================================================
# Include needed submakes
#==============================================================================
MAKEFILES	:=$(LXAPI32DEV)\Makes;..\Makes
include $(LXAPI32DEV)\makes\version.mak
include $(LXAPI32DEV)\makes\paths.mak
include $(LXAPI32DEV)\makes\envchk.mak
include $(LXAPI32DEV)\makes\extns.mak
include $(LXAPI32DEV)\include\defconfig

#==============================================================================
# Set some paths and commands
#==============================================================================
ICC	:=ICC.EXE
GCCAOUT:=GCC.EXE
ifeq "$(USEELF)" "1"
OTOOBJ	:=ELFOMF.EXE -Zep -h4
else
OTOOBJ	:=ELFOMF.EXE -h4
endif
#OTOOBJ	:=EMXOMF.EXE -h4
OSTOOBJ	:=EMXOMF.EXE -h4

ALP	:=$(DDK)\Tools\ALP.EXE
GAS	:=AS.EXE
ASELF	:=AS-ELF.EXE
LDELF	:=LD-ELF.EXE
LIBTOOL:=LIB

LNKNOFREEFORMAT:=1

ifeq "$(USELINK386)" "1"
LNKTOOL:=LINK386
else
# With ILINK we must use /NOFReeformat for the driver!
LNKTOOL:=ILINK /NOFReeformat
endif
IMLIB	:=IMPLIB /Nologo
BLDMGR	:=cmd.exe /C $(LXAPI32DEV)\Tools\bldmgr.cmd

ifeq "$(USEELF)" "1"
CC		:=GCC_ELF.EXE
AS		:=AS_ELF.EXE
else
CC		:=GCC.EXE
AS		:=AS.EXE
endif


CRT0			:=$(LX_BASE)\lxlibc\lib\lxcrt0.obj

OBJFIRST		:= $(addsuffix .$(OBJ_EXT),$(SRCFIRST))
OBJFILES		:= $(addsuffix .$(OBJ_EXT),$(SRCFILES))
OBJLAST		:= $(addsuffix .$(OBJ_EXT),$(SRCLAST))
OFIRST			:= $(addsuffix .$(O_EXT),$(SRCFIRST))
OFILES			:= $(addsuffix .$(O_EXT),$(SRCFILES))
OLAST			:= $(addsuffix .$(O_EXT),$(SRCLAST))

OBJSALL		:= $(OBJFIRST) $(OBJFILES) $(OBJLAST)
OSALL			:= $(OFIRST) $(OFILES) $(OLAST)

LX_MAINMAKE	:=$(LX_BASE)\makes\mainmake.mak

ifeq	"$(strip $(DRV_BIN))" ""
DRV_BIN		:=$(DRV_BASE)\bin
endif
DRV_LIB		:=$(DRV_BASE)\lib
DRV_EXECS	:=$(LX_ROOT)\sbin
DRV_MODULES	:=$(LX_ROOT)\sbin
ifeq	"$(strip $(DRV_OBJ))" ""
DRV_OBJ		:=bin
endif
DRV_INCLUDE	:=..\include;$(LXAPI32DEV)\include
DRV_INC		:=$(DRV_BASE)\inc
LX_LIB			:=$(LX_BASE)\lib
LX_LIBOBJPATH	:=$(LX_BASE)\lxapilib\bin
LXC_LIB		:=$(LX_BASE)\lxlibc\lib

SBINPATH		:=$(LX_BASE)\root\sbin
MODPROBEPATH	:=$(SBINPATH)\modprobe

ASMINCFILES	:=$(wildcard $(LXAPI32DEV)/inc/*.inc)

#==============================================================================
# Some usefull Macros and Targets
#==============================================================================
# ShowHelloMsg
# Variable HELLOMSG must be defined
ShowHelloMsg:
	@echo **********************************************************************
	@echo $(HELLOMSG)
	@echo **********************************************************************

# MakeBinDir
# Makes bin subdirectory if not exists
MakeBinDir:
ifneq "$(strip $(OBJSALL))" ""
	@if not exist bin @md bin
endif

# CleanCommonFiles
# Cleans up some common files
CleanCommonFiles:
	@echo Cleanup...
	@if exist $(DRV_SRC)\*.err del $(DRV_SRC)\*.err
	@if exist $(DRV_OBJ)\*.o del $(DRV_OBJ)\*.o
	@if exist $(DRV_OBJ)\*.i del $(DRV_OBJ)\*.i
	@if exist $(DRV_OBJ)\*.s del $(DRV_OBJ)\*.s
	@if exist $(DRV_OBJ)\*.obj del $(DRV_OBJ)\*.obj
	@if exist $(DRV_OBJ)\*.lrf del $(DRV_OBJ)\*.lrf
	@if exist $(DRV_OBJ)\*.lst del $(DRV_OBJ)\*.lst
	@if exist $(DRV_OBJ)\*.lss del $(DRV_OBJ)\*.lss
	@if exist $(DRV_OBJ)\*.ppp del $(DRV_OBJ)\*.ppp
	@if exist $(DRV_BIN)\*.def del $(DRV_BIN)\*.def
	@if exist $(DRV_BIN)\*.map del $(DRV_BIN)\*.map

COMMONDEPENDENCIES	:=$(LX_MAINMAKE) $(DRV_BASE)\build.cmd $(LX_BASE)\LXApiEnv.CMD makefile

#==============================================================================
# Includes
#==============================================================================
CINCLUDES		:=$(strip $(PRE_INCLUDES) -I$(LXAPI32DEV)\Include -I$(LXAPI32DEV)\Include\asm\mach-default -I$(GCC)\Include -I$(DDK)\BASE\H $(POST_INCLUDES))
AINCLUDES		:=-Fdi:$(DRV_INC) -Fdi:$(LX_BASE)\lxapilib -Fdi:$(DDK)\BASE\inc -Fdi:$(LX_BASE)\Inc
GAINCLUDES		:=$(CINCLUDES)

#==============================================================================
# Defines
#==============================================================================
ifeq "$(strip $(LXDEFKERNEL))" ""
CDEFINES		:=-DTARGET_OS2 -DBUILD_LEVEL=$(BUILD_LEVEL)
else
CDEFINES		:=-D__KERNEL__ -DTARGET_OS2 -DBUILD_LEVEL=$(BUILD_LEVEL)
endif
ADEFINES		:=
GADEFINES		:=
ifdef LXDEBUG
CDEFINES		+=-DLXDEBUG
endif
ifdef LXKERNEL
ifneq ($(DEVTYPE),LXAPILIB)
ADEFINES		+=-D:LXKERNEL
CDEFINES		+=-DLXKERNEL
GADEFINES		+=--defsym LXKERNEL=1
endif
endif

ifeq "$(strip $(USEELF))" "1"
CDEFINES		+=-DTARGET_OS2_ELF
ADEFINES		+=-D:TARGET_OS2_ELF
else
CDEFINES		+=-DTARGET_OS2_AOUT
ADEFINES		+=-D:TARGET_OS2_AOUT
endif

ifeq "$(strip $(OLDGCC))" "1"
CDEFINES		+=-DOS2OLDGCC
endif

CDEFINES		:=$(strip $(strip $(PRE_CDEFINES)) $(CDEFINES))

#==============================================================================
# Flags
#==============================================================================
CBEGINFLAGS	:=
ifneq	"$(strip $(LXOMF))" ""
#CBEGINFLAGS	+=-Zomf
endif
ifeq "$(USEELF)" "1"
CWARNFLAGS		:=-Wimplicit -Wmissing-braces -Wparentheses -Wreturn-type -Wswitch -Wtrigraphs -Wunknown-pragmas -Wcast-align -Werror
else
CWARNFLAGS		:=-Wimplicit -Wmissing-braces -Wparentheses -Wsequence-point -Wreturn-type -Wswitch -Wtrigraphs -Wunknown-pragmas -Wcast-align -Wmissing-noreturn -Werror
endif
CALIGNV	:=	4
ifeq "$(USEELF)" "1"
CALIGNFLAGS	:=-align-functions=4 -align-loops=4 -align-jumps=4 -align-labels=4
else
CALIGNFLAGS	:=-falign-functions=4 -falign-loops=4 -falign-jumps=4 -falign-labels=4
#CALIGNFLAGS	:=
endif
ifeq "$(strip $(LXNOUSEREGPARMS))" ""
#CREGPARMS		:=-mregparm=3
CREGPARMS		:=
else
CREGPARMS		:=
endif
ifdef LXDEFKERNEL
CBLTIN			:=-fno-builtin -ffreestanding
else
CBLTIN			:=-fno-builtin -ffreestanding
endif
#CFLAGS			:=$(CREGPARMS) -mcpu=i586 -march=i586 -fpack-struct $(CBLTIN)
CFLAGS			:=$(CREGPARMS) $(CBLTIN)
AFLAGS			:=-Mb +Mwe +Li +Sv:M510 -LcX -LcY -LcL -LcC -LcF -LcO
LIBFLAGS		:=/Nologo /Noignorecase /Noextdictionary
ifdef USELINK386
LNKFLAGS		:=/NOLogo /NOExtdictionary /NODefaultlibrarysearch /NOIgnorecase /MAP /NOPACKC /NOPACKD
else
#LNKFLAGS		:=/NOLogo /NOExtdictionary /NODefaultlibrarysearch /OLDcpp /MAP /NOOPTF /NOPACKC /NOPACKD /exepack:2
LNKFLAGS		:=/NOLogo /NOExtdictionary /NODefaultlibrarysearch /OLDcpp /MAP /NOOPTF /NOPACKC /NOPACKD
endif
GAFLAGS		:=
ifdef LXLINUXDEBUG
 ifdef LXDEBUG
  ifndef CONFIG_LXLINUXDEBUG
   undef LXDEBUG
  endif
 endif
endif

OPTIMIZED_LIBS	:=
ifeq	($(TARGET),$(findstring $(TARGET),$(OPTIMIZED_LIBS)))
 LXFORCEO		:= 1
endif

CFLAGS_R		:=
GAFLAGS_R		:=
LNKFLAGS_R		:=
AFLAGS_R		:=
CWARNFLAGS_R	:=

CFLAGS_O		:=-O2 -fomit-frame-pointer -frerun-loop-opt -fregmove -freduce-all-givs
GAFLAGS_O		:=
LNKFLAGS_O		:=
AFLAGS_O		:=
CWARNFLAGS_O	:=

CFLAGS_D		:=-O0 -g3
GAFLAGS_D		:=
LNKFLAGS_D		:=/DEbug
AFLAGS_D		:=+Od +Odl +Ods
CWARNFLAGS_D	:=-Wuninitialized

ifdef LXFORCEO
 CFLAGS_R		:=-O2 -fomit-frame-pointer -frerun-loop-opt -fregmove -freduce-all-givs
 GAFLAGS_R		:=--gstabs
else
 ifdef LXDEBUG
  CFLAGS_R		:=-O0 -g3
  LNKFLAGS_R	:=/DEbug
  AFLAGS_R		:=+Od +Odl +Ods
 else
  CWARNFLAGS_R	:=-Wuninitialized
  CFLAGS_R		:=-O2 -fomit-frame-pointer -frerun-loop-opt -fregmove -freduce-all-givs
  GAFLAGS_R	:=--gstabs
 endif
endif

CFLAGS			:= $(CFLAGS_R) $(CFLAGS)
CWARNFLAGS		:= $(CWARNFLAGS_R) $(CWARNFLAGS)
AFLAGS			:= $(AFLAGS_R) $(AFLAGS)
LNKFLAGS		:= $(LNKFLAGS_R) $(LNKFLAGS)
GAFLAGS		:= $(GAFLAGS_R) $(GAFLAGS)

#==============================================================================
# Final commands
#==============================================================================
CC				+=$(strip $(CBEGINFLAGS) $(CFLAGS) $(CWARNFLAGS) $(CALIGNFLAGS) $(CDEFINES) $(CINCLUDES))
GCCAOUT		+=$(strip $(CBEGINFLAGS) $(CFLAGS) $(CWARNFLAGS) $(CALIGNFLAGS) $(CDEFINES) $(CINCLUDES))
ALP			+=$(strip $(AFLAGS) $(ADEFINES) $(AINCLUDES))
GAS			+=$(strip $(GAFLAGS) $(GADEFINES) $(GAINCLUDES))
LIBTOOL		+=$(strip $(LIBFLAGS))

LNKTOOL		+=$(strip $(LNKFLAGS))

#==============================================================================
# Standard files
#==============================================================================
ifdef AOUT_TARGET
EXEFILE		:=$(DRV_EXECS)\$(TARGET)
MODULEFILE		:=$(DRV_MODULES)\$(TARGET).o
else
EXEFILE		:=$(DRV_EXECS)\$(TARGET).exe
MODULEFILE		:=$(DRV_MODULES)\$(TARGET).o
endif
ifeq	"$(strip $(LIBFILE))" ""
LIBFILE		:=$(DRV_LIB)\$(TARGET).$(LIB_EXT)
endif
ifeq	"$(strip $(DEPFILE))" ""
ifeq	"$(strip $(LXCLEAN))" ""
DEPFILE		:=$(DRV_OBJ)\$(TARGET).dep
endif
endif

.PHONY:		all
ifdef DIRS
.PHONY:		$(DIRS)
endif

#==============================================================================
# Correcting some variables
#==============================================================================
LIB			:=$(subst /,\,$(LIB))

#==============================================================================
# Rules
#==============================================================================
vpath %.i .\$(DRV_OBJ)
vpath %.s .\$(DRV_OBJ)
vpath %.o .\$(DRV_OBJ)
vpath %.obj .\$(DRV_OBJ)

ifndef OWNCTOOBJ
%.$(OBJ_EXT): %.c $(COMMONDEPENDENCIES)
	@echo --------------------- $@
	@if exist *.err @del *.err
	$(CC) -c $< -o $(DRV_OBJ)\$*.$(O_EXT) -DKBUILD_BASENAME=$(subst $(comma),_,$(subst -,_,$(*F))) -Wa,-ahls=$(DRV_OBJ)\$*.lst 2> $*.err
	$(OTOOBJ) $(DRV_OBJ)\$*.$(O_EXT) 2>> $*.err
	@if exist *.err @del *.err
	@$(SHELL) /C ExtrSyms.cmd $(DRV_OBJ)\$*
endif

ifndef OWNSTOOBJ
%.$(OBJ_EXT): %.S $(COMMONDEPENDENCIES)
	@echo --------------------- $@
	@if exist *.err @del *.err
	$(GCCAOUT) -c -dD -nostdinc $< -o $(DRV_OBJ)\$*.$(O_EXT) -D__ASSEMBLY__ -DKBUILD_BASENAME=$(subst $(comma),_,$(subst -,_,$(*F))) -Wa,-ahls=$(DRV_OBJ)\$*.lst 2> $*.err
	$(OSTOOBJ) $(DRV_OBJ)\$*.$(O_EXT) 2>> $*.err
	@if exist *.err @del *.err
	@$(SHELL) /C ExtrSyms.cmd $(DRV_OBJ)\$*
endif

ifndef OWNASMTOOBJ
%.obj: %.asm $(COMMONDEPENDENCIES) $(ASMINCFILES)
	@echo --------------------- $@
	@if exist *.err @del *.err
	$(ALP) $< -Fdo:$(DRV_OBJ) -Fdl:$(DRV_OBJ) -Fl:$(DRV_OBJ)\$*.lst > $*.err
	@if exist *.err @del *.err
	@$(SHELL) /C ExtrSyms.cmd $(DRV_OBJ)\$*
endif

ifndef OWNOTOOBJ
%.obj: %.o $(COMMONDEPENDENCIES)
	@echo --------------------- $@
	$(OTOOBJ) $(DRV_OBJ)\$*.$(O_EXT)
endif

ifndef OWNIMLIB
%.lib: %.def $(COMMONDEPENDENCIES)
	@echo --------------------- $@
	$(IMLIB) $@ $<
endif
