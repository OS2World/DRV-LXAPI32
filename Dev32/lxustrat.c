/* $Id: lxustrat.c,v 1.4 2005/03/22 22:34:30 smilcke Exp $ */

/*
 * lxustrat.c
 * Autor:               Stefan Milcke
 * Erstellt am:         01.03.2005
 * Letzte Aenderung am: 19.03.2005
 *
*/

#include <lxcommon.h>
#include <DEVRP.H>

extern ULONG LX_UStratIOCtl(struct LX_RP* _rp);

//-------------------------------- LX_UStratOk ---------------------------------
static WORD32 LX_UStratOk(struct LX_RP* rp)
{
 return RPDONE;
}

//------------------------------- LX_UStratError -------------------------------
static WORD32 LX_UStratError(struct LX_RP* rp)
{
 return RPERR_COMMAND | RPDONE;
}

typedef WORD32 (*RPHandler)(struct LX_RP* rp);
static RPHandler lx_UStratDispatch[] =
{
  LX_UStratOk,          // 00 (BC): Initialization
  LX_UStratError,       // 01 (B ): Media check
  LX_UStratError,       // 02 (B ): Build BIOS parameter block
  LX_UStratError,       // 03 (  ): Unused
  LX_UStratError,       // 04 (BC): Read
  LX_UStratError,       // 05 ( C): Nondestructive read with no wait
  LX_UStratError,       // 06 ( C): Input status
  LX_UStratError,       // 07 ( C): Input flush
  LX_UStratError,       // 08 (BC): Write
  LX_UStratError,       // 09 (BC): Write verify
  LX_UStratError,       // 0A ( C): Output status
  LX_UStratError,       // 0B ( C): Output flush
  LX_UStratError,       // 0C (  ): Unused
  LX_UStratOk,          // 0D (BC): Open
  LX_UStratOk,          // 0E (BC): Close
  LX_UStratError,       // 0F (B ): Removable media check
  LX_UStratIOCtl,       // 10 (BC): IO Control
  LX_UStratError,       // 11 (B ): Reset media
  LX_UStratError,       // 12 (B ): Get logical unit
  LX_UStratError,       // 13 (B ): Set logical unit
  LX_UStratOk,          // 14 ( C): Deinstall character device driver
  LX_UStratError,       // 15 (  ): Unused
  LX_UStratError,       // 16 (B ): Count partitionable fixed disks
  LX_UStratError,       // 17 (B ): Get logical unit mapping of fixed disk
  LX_UStratError,       // 18 (  ): Unused
  LX_UStratError,       // 19 (  ): Unused
  LX_UStratError,       // 1A (  ): Unused
  LX_UStratError,       // 1B (  ): Unused
  LX_UStratOk,          // 1C (BC): Notify start or end of system shutdown
  LX_UStratError,       // 1D (B ): Get driver capabilities
  LX_UStratError,       // 1E (  ): Unused
  LX_UStratOk           // 1F (BC): Notify end of initialization
};

//-------------------------------- LX_UStrategy --------------------------------
WORD32 fastcall LX_UStrategy(struct LX_RP* rp)
{
 WORD32 rc;
 if (rp->Command < sizeof(lx_UStratDispatch)/sizeof(lx_UStratDispatch[0]))
 {
  LX_enter_utl_current();
  rc=(lx_UStratDispatch[rp->Command](rp));
  if(rp->Command==0x0e) // Close ?
   LX_leave_utl_current(LXLEAVE_UTL_FREEME);
  else
   LX_leave_utl_current(0);
 }
 else
  rc=(RPERR_COMMAND | RPDONE);
 return rc;
}
