/* $Id: DEVRP.H,v 1.9 2006/02/16 23:07:16 smilcke Exp $ */

/*
 * DEVRP.C  (for GCC)
 * Autor:               Stefan Milcke
 * Erstellt am:         11.03.2004
 * Letzte Aenderung am: 12.01.2006
 *
*/

#ifndef DEVRP_H_INCLUDED
#define DEVRP_H_INCLUDED

// Include the fundamental type definitions

#pragma pack(1)

// REQUEST PACKET
struct LX_RP
{
 unsigned char       Length;        // Length of request packet
 unsigned char       Unit;          // Unit code (B)
 unsigned char       Command;       // Command code
 unsigned short      Status;        // Status code
 unsigned char       Flags;         // Flags
 unsigned char       Reserved[3];   // Reserved
 unsigned long       Link;          // Link to next request packet in queue
 union
 {
// Request packet 00 (BC): Initialization
  struct
  {
   unsigned char     Reserved;      // Unused
   unsigned long     DevHlp;        // Address of DevHlp entry point
   unsigned long     Args;          // Command-line arguments
   unsigned char     Drive;         // Drive number of first unit (B)
  } RPINITIN;
  struct
  {
   unsigned char     Units;         // Number of supported units (B)
   unsigned short    FinalCS;       // Offset of last resident code
   unsigned short    FinalDS;       // Offset of last resident data
   unsigned long     Bpb;           // BIOS parameter block (B)
  } RPINITOUT;
// Request packet 01 (B ): Media check
  struct
  {
   unsigned char     Media;
   unsigned char     Result;
   unsigned long     PrevVolID;
  } RPMEDIACHECK;
// Request packet 02 (B ): Build BIOS parameter block
  struct
  {
   unsigned char     Media;         // Media descriptor
   unsigned long     Transfer;      // Transfer virtual address
   unsigned long     Bpb;           // Pointer to BPB table
   unsigned char     Drive;         // Drive number
  }RPBUILDBPB;
// Request packet 04 (BC): Read
// Request packet 08 (BC): Write
// Request packet 09 (BC): Write verify
  struct
  {
   unsigned char     Media;         // Media descriptor
   unsigned long     Transfer;      // Transfer physical address
   unsigned short    Count;         // Number of bytes (C) or sectors (B)
   unsigned long     Start;         // Starting sector number (B)
   unsigned short    FileID;        // System file number
  } RPRWV;
// Request packet 05 ( C): Nondestructive read with no wait
  struct
  {
   unsigned char     Data;          // Return character
  }RPREADNOWAIT;
// Request packet 06 ( C): Input status
// Request packet 0A ( C): Output status
/*
  struct _RPIOSTATUS
  {
  };
*/
// Request packet 07 ( C): Input flush
// Request packet 0B ( C): Output flush
/*
  struct _RPIOFLUSH
  {
  };
*/
// Request packet 0D (BC): Open
// Request packet 0E (BC): Close
  struct
  {
   unsigned short    FileID;        // System file number
  }RPOPENCLOSE;
// Request packet 0F (B ): Removable media check
/*
  struct _RPREMOVABLE
  {
  } RPREMOVABLE;
*/
// Request packet 10 (BC): IO Control
  struct _RPIOCTL
  {
   unsigned char     Category;      // Category code
   unsigned char     Function;      // Function code
   unsigned long     ParmPacket;    // Parameter packet
   unsigned long     DataPacket;    // Data packet
   unsigned short    FileID;        // System file number
   unsigned short    ParmLength;    // Parameter packet length
   unsigned short    DataLength;    // Data packet length
  }RPIOCTL;
// Request packet 11 (B ): Reset media
/*
  struct _RPRESET
  {
  } RPRESET;
*/
// Request packet 12 (B ): Get logical unit
// Request packet 13 (B ): Set logical unit
/*
  struct _RPUNIT
  {
  } RPUNIT;
*/
// Request packet 14 ( C): Deinstall character device driver
/*
  struct _RPDEINSTALL
  {
  } RPDEINSTALL;
*/
// Request packet 16 (B ): Count partitionable fixed disks
  struct
  {
   unsigned char     Count;         // Number of partitionable fixed disks
   unsigned long     Reserved;      // Reserved
  }RPPARTITIONABLE;
// Request packet 17 (B ): Get logical unit mapping of fixed disk
  struct
  {
   unsigned long     Units;         // Bitmap of supported logical units
   unsigned long     Reserved;      // Reserved
  }RPUNITMAP;
// Request packet 1C (BC): Notify start or end of system shutdown
  struct
  {
   unsigned char     Function;      // Shutdown code: 0 = start, 1 = end
   unsigned long     Reserved;      // Reserved
  }RPSHUTDOWN;
// Request packet 1D (B ): Get driver capabilities
  struct
  {
   unsigned char     Reserved[3];   // Reserved
   unsigned long     DCS;           // Pointer to driver capabilities structure
   unsigned long     VCS;           // Pointer to volume characteristics structure
  }RPDRIVERCAPS;
// Request packet 1F (BC): Notify end of initialization
/*
  struct
  {
  } RPINITCOMPLETE;
*/
 }RPBODY;
};

// RP Result codes

#define RPERR                 0x8000      // Error
#define RPDEV                 0x4000      // Device-specific error code
#define RPBUSY                0x0200      // Device is busy
#define RPDONE                0x0100      // Command complete

#define RPERR_PROTECT         0x8000      // Write-protect error
#define RPERR_UNIT            0x8001      // Unknown unit
#define RPERR_READY           0x8002      // Not ready
#define RPERR_COMMAND         0x8003      // Unknown command
#define RPERR_CRC             0x8004      // CRC error
#define RPERR_LENGTH          0x8005      // Bad request length
#define RPERR_SEEK            0x8006      // Seek error
#define RPERR_FORMAT          0x8007      // Unknown media
#define RPERR_SECTOR          0x8008      // Sector not found
#define RPERR_PAPER           0x8009      // Out of paper
#define RPERR_WRITE           0x800A      // Write fault
#define RPERR_READ            0x800B      // Read fault
#define RPERR_GENERAL         0x800C      // General failure
#define RPERR_DISK            0x800D      // Disk change
#define RPERR_MEDIA           0x8010      // Uncertain media
#define RPERR_INTERRUPTED     0x8011      // Call interrupted (character)
#define RPERR_MONITOR         0x8012      // Monitors unsupported
#define RPERR_PARAMETER       0x8013      // Invalid parameter
#define RPERR_USE             0x8014      // Device in use
#define RPERR_INIT            0x8015      // Non-critical init failure

/******************************************************************************/
/* Some macros and inline functions for IOCtl (smilcke)                       */
/******************************************************************************/
void* LX_virtToLin(void *addr);
void* LX_verifyToLin(unsigned long addr,unsigned short length,unsigned char type);

#define DPFROMRP(rp) (LX_virtToLin((void *)(rp)->RPBODY.RPIOCTL.DataPacket))
#define PAFROMRP(rp) (LX_virtToLin((void *)(rp)->RPBODY.RPIOCTL.ParmPacket))

#define VDPFROMRP(rp,len,accessType) \
(LX_verifyToLin((unsigned long)(rp)->RPBODY.RPIOCTL.DataPacket,len,accessType))
//(DevVerifyAccess((unsigned long)rp->DataPacket,len,accessType) ? NULL : DPFROMRP(rp))

#define VPAFROMRP(rp,len,accessType) \
(LX_verifyToLin((unsigned long)(rp)->RPBODY.RPIOCTL.ParmPacket,len,accessType))
//(DevVerifyAccess((unsigned long)rp->ParmPacket,len,accessType) ? NULL : PAFROMRP(rp))

#define VERIFY_READ        0
#define VERIFY_READWRITE   1

#define DATAPACKET(name,type,rp,accessType) \
type name=(type)VDPFROMRP((rp),((rp)->RPBODY.RPIOCTL.DataLength),(accessType))

#define PARMPACKET(name,type,rp,accessType) \
type name=(type)VPAFROMRP((rp),((rp)->RPBODY.RPIOCTL.ParmLength),(accessType))

#endif // DEVRP_H_INCLUDED
