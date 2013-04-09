# $Id: makefile,v 1.9 2005/12/26 23:42:03 smilcke Exp $

TARGET     = lxcrt0
USEELF:=0

#===================================================================
#   List of source files
#===================================================================
SRCFIRST	:=
SRCFILES	:=
SRCLAST	:=

#OWNMAKELIB		:= 1
LXOMF				:= 1
LXNOUSEREGPARMS	:= 1

include $(LXAPI32DEV)\makes\paths.mak
include $(LXAPI32DEV)\makes\extns.mak

LIBFILE :=$(DRV_BASE)\lxlibc\lib\lxcrt0.obj

include $(LXAPI32DEV)\makes\mainmake.mak

ifdef LXDEBUG
ICC	  :=ICC.EXE /Mc /Ss+ /Ti+ /C+
else
ICC	  :=ICC.EXE /Mc /Ss+ /G5 /Gd- /Ge+ /Gm+ /O+ /C+
endif
ICC	  +=/I$(LXAPI32DEV)\Include /I$(LXAPI32DEV)\lxlibc\include

all: makefile $(LIBFILE)

$(LIBFILE): lxcrt0.c
	$(ICC) /Fo$(LIBFILE) lxcrt0.c
