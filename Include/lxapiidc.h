/* $Id: lxapiidc.h,v 1.32 2004/12/16 23:03:17 smilcke Exp $ */

/*
 * lxapiidc.h
 * Autor:               Stefan Milcke
 * Erstellt am:         27.12.2001
 * Letzte Aenderung am: 09.11.2004
 *
*/

#ifndef LXAPIIDC_H_INCLUDED
#define LXAPIIDC_H_INCLUDED

/* defines for OS/2 include */

/* Standard headers */

/* Application headers */

/* OS/2 Headers */
#pragma pack(1)
#include <lxrmcall.h>

#define LXAPI_VERSION                           9

#define LXIDC_ERR_NONE                          (0)
#define LXIDC_ERR_VERSION                       (1)

//---------------------------------- Thunking ----------------------------------
#define LXIDCCMD_GETTHUNKAPI                    0
typedef struct _LXIDC_GETTHUNKAPI
{
 unsigned long required_version;       // IN: Required version
 unsigned long actual_version;         // OUT: Actual version
 VOID  (*LXA_FixSelDPL)(void);
 VOID  (*LXA_RestoreSelDPL)(void);
 ULONG (*LXA_StackAlloc)(void);
 VOID  (*LXA_StackFree)(ULONG stackaddr);
 ULONG (*LXA_IrqStackAlloc)(void);
 VOID  (*LXA_IrqStackFree)(ULONG addr32);
}LXIDC_GETTHUNKAPI,*PLXIDC_GETTHUNKAPI;

#endif //LXAPIIDC_H_INCLUDED
