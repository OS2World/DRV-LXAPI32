/* $Id: lxlcvars.c,v 1.3 2005/04/18 22:28:43 smilcke Exp $ */

/*
 * lxlcvars.c
 * Autor:               Stefan Milcke
 * Erstellt am:         14.03.2005
 * Letzte Aenderung am: 10.04.2005
 *
*/

#define INCL_DOS

#include <lxlibc.h>

HFILE lxapi_rt_hfile=0;
ULONG lxapi_sysstate=LXSYSSTATE_NOT_INITIALIZED | LXSYSSTATE_NOT_OPERABLE;
