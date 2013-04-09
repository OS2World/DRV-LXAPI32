# $Id: envchk.mak,v 1.9 2005/04/18 22:28:39 smilcke Exp $

#===================================================================
# Check environment
#===================================================================
ifndef LX_BASE
error No LX_BASE paths defined.
endif

ifndef DDK
error DDK is not defined.
endif

ifndef DRV_BASE
error No DRV_BASE paths defined.
endif

ifndef GCC
error No GCC paths defined.
endif

ifndef EMXUTILS
error No EMXUTILS paths defined.
endif
