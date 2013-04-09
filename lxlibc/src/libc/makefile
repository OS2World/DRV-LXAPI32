# $Id: makefile,v 1.13 2005/12/26 23:42:04 smilcke Exp $

TARGET     = lxlibc
USEELF:=0

#===================================================================
#   Setup sources
#===================================================================

#===================================================================
#   List of source files
#===================================================================
WRAPPERS	:=	lxlcio

SRCFIRST	:=
SRCFILES	:=	lxlcalloca \
				lxlcconvert \
				lxlcdir \
				lxlcerrno \
				lxlcfilehandle \
				lxlcvars \
				lxlcmodule \
				lxlcsys \
				lxlcsyscalls \
				lxlctime \
				$(WRAPPERS) \
				lxinitrt
SRCLAST		:=

#OWNMAKELIB		:= 1
LXOMF				:= 1
LXNOUSEREGPARMS	:= 1

include $(LXAPI32DEV)\makes\paths.mak
include $(LXAPI32DEV)\makes\extns.mak

LIBFILE :=$(DRV_BASE)\lxlibc\lib\lxlibc_s.$(LIB_EXT)

include $(LXAPI32DEV)\makes\libcmake.mak

all: makefile $(LIBFILE)
