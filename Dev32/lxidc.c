/* $Id: lxidc.c,v 1.34 2006/01/05 23:48:14 smilcke Exp $ */

/*
 * lxidc.c
 * Autor:               Stefan Milcke
 * Erstellt am:         08.11.2001
 * Letzte Aenderung am: 29.12.2005
 *
*/


#include <lxcommon.h>

#include <lxapiidc.h>

/*
extern void LXA_FixSelDPL(void);
extern void LXA_RestoreSelDPL(void);
extern ULONG LXA_StackAlloc(void);
extern void LXA_StackFree(ULONG stackaddr);
extern ULONG LXA_StackAlloc(void);
extern void LXA_StackFree(ULONG stackaddr);
extern ULONG LXA_IrqStackAlloc(void);
extern void LXA_IrqStackFree(ULONG stackaddr);
*/

#define SETTOLXAPI(name) ((pApi->name)=(name))

//---------------------------------- IDCEntry ----------------------------------
WORD32 IDCEntry(ULONG cmd, ULONG packet)
{
 WORD32 rc=LXIDC_ERR_NONE;
 LX_enter_current();
 if(cmd==LXIDCCMD_GETTHUNKAPI)
 {
  PLXIDC_GETTHUNKAPI pApi=(PLXIDC_GETTHUNKAPI)packet;
  if(LXAPI_VERSION<pApi->required_version)
   return LXIDC_ERR_VERSION;
  pApi->actual_version=LXAPI_VERSION;
/*
  SETTOLXAPI(LXA_FixSelDPL);
  SETTOLXAPI(LXA_RestoreSelDPL);
  SETTOLXAPI(LXA_StackAlloc);
  SETTOLXAPI(LXA_StackFree);
  SETTOLXAPI(LXA_IrqStackAlloc);
  SETTOLXAPI(LXA_IrqStackFree);
*/
 }
 LX_leave_current();
 return rc;
}
