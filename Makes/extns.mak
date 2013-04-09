# $Id: extns.mak,v 1.2 2005/12/07 23:46:02 smilcke Exp $

OBJ_EXT	:=	obj
O_EXT	:=	o

ifneq	"$(strip $(LXOMF))" ""
LIB_EXT	:=	lib
else
LIB_EXT	:=	a
endif
