# $Id: makefile,v 1.9 2005/03/15 23:37:57 smilcke Exp $

#==============================================================================
# Predefinitions
#==============================================================================
TARGET 	= IMPORTLIBS
HELLOMSG	= Building Import Libraries

#==============================================================================
# Include Main Makefile
#==============================================================================
include ..\makes\mainmake.mak

#==============================================================================
# Setup environment
#==============================================================================
.PHONY:	all

LIBS	=	impdos.lib impkee.lib

all: ShowHelloMsg $(LIBS) makefile $(COMMONDEPENDENCIES)

clean: ShowHelloMsg
	@echo Cleanup...
	@for %f in ($(LIBS)) do @if exist %f @del %f
