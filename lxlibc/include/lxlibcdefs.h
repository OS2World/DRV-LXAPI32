/* $Id: lxlibcdefs.h,v 1.5 2005/04/18 22:28:40 smilcke Exp $ */

/*
 * lxlibcdefs.h
 * Autor:               Stefan Milcke
 * Erstellt am:         29.03.2005
 * Letzte Aenderung am: 14.04.2005
 *
*/

#ifndef LXLIBCDEFS_H_INCLUDED
#define LXLIBCDEFS_H_INCLUDED

#include <os2emx.h>

//#define LXAPIENTRY   __cdecl
#define LXAPIENTRY
#define LXAPISYSCRET unsigned long

#define LXAPI_INLINE __attribute__((always_inline))

#endif //LXLIBCDEFS_H_INCLUDED
