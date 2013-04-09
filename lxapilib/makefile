# $Id: makefile,v 1.13 2005/02/06 22:49:27 smilcke Exp $

#===================================================================
# Makefile for BT32 driver
#===================================================================
TARGET     = lxapilib
HELLOMSG	= Building $(TARGET)
#==============================================================================
# Setup subdirectories
#==============================================================================
.PHONY:	all

#==============================================================================
# Include Main Makefile
#==============================================================================
include ..\makes\mainmake.mak

#===================================================================
#
#   List of source files
#
#===================================================================
OBJSFIRST:=
OBJS     :=lxapistrt.obj\
           lxapilib.obj\
           lxapihlp.obj
OBJLAST  :=

OBJS	  :=$(OBJSFIRST) $(OBJS) $(OBJSLAST)

LNKFILE = $(DRV_OBJ)\$(TARGET).lrf

#===================================================================
#
#   Specific dependencies
#
#===================================================================
all: ShowHelloMsg MakeBinDir $(OBJSFIRST) $(OBJS) $(OBJSLAST) makefile $(COMMONDEPENDENCIES)

clean:
	@if exist $(DRV_OBJ)\*.obj del $(DRV_OBJ)\*.obj
	@if exist $(DRV_OBJ)\*.lrf del $(DRV_OBJ)\*.lrf
	@if exist $(DRV_OBJ)\*.lst del $(DRV_OBJ)\*.lst
	@if exist $(DRV_SRC)\*.err del $(DRV_SRC)\*.err
	@if exist $(TARGET).lib del $(TARGET).lib
