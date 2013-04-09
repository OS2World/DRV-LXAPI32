/* $Id: DEVTYPE.H,v 1.6 2006/02/28 19:46:51 smilcke Exp $ */

/*
 * DEVTYPE.H (for GCC)
 * Autor:               Stefan Milcke
 * Erstellt am:         15.03.2004
 * Letzte Aenderung am: 20.02.2006
 *
*/

#ifndef DEVTYPE_H_INCLUDED
#define DEVTYPE_H_INCLUDED

#include <os2.h>

// Standard types
typedef  unsigned long  DWORD;
typedef  unsigned short WORD16;
typedef  unsigned long  WORD32;
typedef  void           (*FUNCTION)(void);
typedef  ULONG          FARPTR16;

// Pointer type for physical addresses
typedef  WORD32      PHYSICAL;

// Pointer type for linear addresses
typedef  BYTE*    LINEAR;

// Pointer types for virtual addresses
typedef  WORD16      SEGMENT;
typedef  WORD16      OFFSET;
typedef  WORD32      VIRTUAL;

// Selector type for local and global descriptor tables
//typedef   WORD16         SEL; // (defined in OS2EMX.H)

// Locked segment handle type
typedef  WORD32         HLOCK;

// Context hook handle type
typedef  WORD32         HCONTEXT;

// Some usefull macros
#define MAKE_FP16(sel, offset)   ((sel << 16) | (offset & 0xffff) )

// Offsets for the end of the resident code and data segments
#define SEG_MAKE_OFF(n) ((WORD32)&(n))
#define OFF_STRT_TEXT32 ((WORD32)&_SEGSTRT_TEXT32)
#define OFF_END_TEXT32  ((WORD32)&_SEGEND_TEXT32)

#define OFF_STRT_DATA32 ((WORD32)&_SEGSTRT_DATA32)
#define OFF_END_DATA32  ((WORD32)&_SEGEND_BSS32)

extern BYTE _SEGSTRT_TEXT32;
extern BYTE _SEGEND_TEXT32;

extern BYTE _SEGSTRT_DATA32;
extern BYTE _SEGEND_DATA32;

extern BYTE _SEGSTRT_BSS32;
extern BYTE _SEGEND_BSS32;

#endif // DEVTYPE_H_INCLUDED
