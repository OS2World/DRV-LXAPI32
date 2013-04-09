/* $Id: lxsecure.h,v 1.2 2005/01/28 23:46:04 smilcke Exp $ */

/*
 * lxsecure.h
 * Autor:               Stefan Milcke
 * Erstellt am:         15.10.2002
 * Letzte Aenderung am: 20.01.2005
 *
*/

#ifndef SECURE_H_INCLUDED
#define SECURE_H_INCLUDED

/* defines for OS/2 include */

/* Standard headers */

/* Application headers */

/* OS/2 Headers */

//#define PFINDPARMS   PVOID
typedef struct
{
   PSZ      pszPath;    // well formed path
   ULONG    ulHandle;   // search handle
   ULONG    rc;         // rc user got from findfirst
   PUSHORT  pResultCnt; // count of found files
   USHORT   usReqCnt;   // count user requested
   USHORT   usLevel;    // search level
   USHORT   usBufSize;  // user buffer size
   USHORT   fPosition;  // use position information?
   PCHAR    pcBuffer;   // ptr to user buffer
   ULONG    Position;   // Position for restarting search
   PSZ      pszPosition;// file to restart search with
} FINDPARMS, *PFINDPARMS;

#define SECENTRY  __attribute__((regparm(0)))
#define SECCALL   ULONG SECENTRY

struct SecExp_s
{
 USHORT seVersionMajor;
 USHORT seVersionMinor;
 SECCALL (*SecHlpRead)(ULONG sfn,PULONG pcbBytes,PUCHAR pBuffer,ULONG p16Addr,ULONG offset);
 SECCALL (*SecHlpWrite)(ULONG sfn,PULONG pcbBytes,PUCHAR pBuffer,ULONG p16Addr,ULONG offset);
 SECCALL (*SecHlpOpen)(PSZ pszFileName,PULONG pSfn,ULONG ulOpenFlag,ULONG ulOpenMode);
 SECCALL (*SecHlpClose)(ULONG sfn);
 SECCALL (*SecHlpQFileSize)(ULONG sfn,PULONG pSize);
 SECCALL (*SecHlpChgFilePtr)(ULONG sfn,LONG offset,ULONG type,PULONG pAbs);
 SECCALL (*SecHlpSFFromSFN)(ULONG sfn);
 SECCALL (*SecHlpFindNext)(PFINDPARMS pParms);
 SECCALL (*SecHlpPathFromSFN)(ULONG sfn);
 ULONG *apDemSVC;
};

extern SECCALL SecHlpRead(ULONG sfn,PULONG pcbBytes,PUCHAR pBuffer,ULONG p16Addr,ULONG offset);
extern SECCALL SecHlpWrite(ULONG sfn,PULONG pcbBytes,PUCHAR pBuffer,ULONG p16Addr,ULONG offset);
extern SECCALL SecHlpOpen(PSZ pszFileName,PULONG pSfn,ULONG ulOpenFlag,ULONG ulOpenMode);
extern SECCALL SecHlpClose(ULONG sfn);
extern SECCALL SecHlpQFileSize(ULONG sfn,PULONG pSize);
extern SECCALL SecHlpChgFilePtr(ULONG sfn,LONG offset,ULONG type,PULONG pAbs);
extern SECCALL SecHlpSFFromSFN(ULONG sfn);
extern SECCALL SecHlpFindNext(PFINDPARMS pParms);
extern SECCALL SecHlpPathFromSFN(ULONG sfn);

#endif //SECURE_H_INCLUDED
