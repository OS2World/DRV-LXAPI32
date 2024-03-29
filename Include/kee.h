/* $Id: kee.h,v 1.6 2004/05/27 20:33:33 smilcke Exp $ */

/* Released to the public domain. All rights perverse */
/*
 * This is not an official IBM header file, and never was.
 * It is published in the hope that some day IBM will document
 * the new KEE32 32 bit driver API properly.
 * There is no warranty that the declarations and symbols are correct
 * and match corresponding official symbols correctly, however the
 * information herein has been collected and analysed carefully. It
 * represents the authors's current knowledge about this API.
 */

#ifndef __KEE_H__
#define __KEE_H__

#define KEEENTRY __attribute__((regparm(0)))

/* The spinlock data type. KEE spinlock functions either put 0 (unused)
 * or 0xff (used) into the variable, and 0xff000000 when the lock is
 * acquired.
 */
typedef ULONG KEESpinLock;

/*ordinal 10 */
APIRET KEEENTRY KernAllocSpinLock(KEESpinLock* sl);

/*ordinal 11*/
/* XXX I suppose this routine should receive a KEESpinLock*, but
 * so far, it just returns NO_ERROR, and not even clearing the Spinlock
 * ownership. I think this is a bug in 14.039F_SMP
 */
APIRET KEEENTRY KernFreeSpinLock(KEESpinLock* sl);

/*ordinal 12*/
VOID KEEENTRY KernAcquireSpinLock(KEESpinLock* sl);

/*ordinal 13*/
VOID KEEENTRY KernReleaseSpinLock(KEESpinLock* sl);

typedef struct {
  ULONG mtx[2];
} KEEMutexLock;

/*ordinal 20*/
APIRET KEEENTRY KernAllocMutexLock(KEEMutexLock* ml);

/*ordinal 21*/
/* XXX Like ordinal 11, I think this should really do something. Actually,
 * it just returns NO_ERROR. At least, it accesses the argument. But
 * it doesn't do anything. Again, likely an error in 14.039F_SMP
 */
APIRET KEEENTRY KernFreeMutexLock(KEEMutexLock* ml);

/*ordinal 22*/
VOID KEEENTRY KernRequestSharedMutex(KEEMutexLock* ml);

/*ordinal 23*/
VOID KEEENTRY KernReleaseSharedMutex(KEEMutexLock* ml);

/*ordinal 24*/
INT KEEENTRY KernTryRequestSharedMutex(KEEMutexLock* ml);

/*ordinal 25*/
VOID KEEENTRY KernRequestExclusiveMutex(KEEMutexLock* ml);

/*ordinal 26*/
VOID KEEENTRY KernReleaseExclusiveMutex(KEEMutexLock* ml);

/*ordinal 27*/
INT KEEENTRY KernTryRequestExclusiveMutex(KEEMutexLock* ml);

#define KEE_BLOCK_NOSIGNALS   0x00000001
#define KEE_BLOCK_SPINLOCK      0x00000002
#define KEE_BLOCK_EXMUTEXLOCK   0x00000004
#define KEE_BLOCK_SHMUTEXLOCK   0x00000008
#define KEE_BLOCK_NOACQUIRE     0x00000010

/*ordinal 30*/
APIRET KEEENTRY KernBlock(ULONG id, ULONG timeout, ULONG flags,
           PVOID ptr, PULONG retdata);

#define KEE_WAKE_SINGLE         0x00000001
#define KEE_WAKE_PRIOINCR       0x00000004
#define KEE_WAKE_RETDATA        0x00000008

/*ordinal 31*/
APIRET KEEENTRY KernWakeup(ULONG id, ULONG flags, PULONG ptr, ULONG retdata);

/*ordinal 40*/
// Returns: flat base of kernel stack in eax
//NOTE: esp and ebp are modified when this function returns
//      --> can cause problems if compiler uses i.e. ebp for storing a value!!
ULONG KEEENTRY KernThunkStackTo16(VOID);

/*ordinal 41*/
// Returns: flat base of kernel stack in edx
//          16 bits stack selector in eax (not during interrupts (bug??))
//NOTE: esp and ebp are modified when this function returns
//      --> can cause problems if compiler uses i.e. ebp for storing a value!!

#ifdef   __WATCOMC__
ULONG KernThunkStackTo32(VOID);
#pragma aux KernThunkStackTo32 "KernThunkStackTo32" modify exact [eax ecx edx] value [edx];
#else
ULONG KEEENTRY KernThunkStackTo32(VOID);
#endif


/*ordinal 42*/
VOID KEEENTRY KernSerialize16BitDD(VOID);

/*ordinal 43*/
VOID KEEENTRY KernUnserialize16BitDD(VOID);

/*ordinal 44*/
VOID KEEENTRY KernArmHook(ULONG hook,ULONG data);

/*ordinal 45*/
APIRET KEEENTRY KernAllocateContextHook(PVOID pfHandler,ULONG dummy,
               PULONG pHook);

/*ordinal 50*/
APIRET KEEENTRY KernCopyIn(PVOID trgt, PVOID src, ULONG size);

/*ordinal 51*/
APIRET KEEENTRY KernCopyOut(PVOID trgt, PVOID src, ULONG size);


/* same bits as with DevHlp_VMAlloc, see explanation there */
#define KEE_VMA_16MB       0x00000001  // Bit 0
#define KEE_VMA_FIXED      0x00000002  // Bit 1
#define KEE_VMA_SWAP       0x00000004  // Bit 2
#define KEE_VMA_CONTIG     0x00000008  // Bit 3
#define KEE_VMA_PHYS       0x00000010  // Bit 4
#define KEE_VMA_PROCESS    0x00000020  // Bit 5
#define KEE_VMA_SGSCONT    0x00000040  // Bit 6
#define KEE_VMA_GETSEL     0x00000080  // Bit 7
#define KEE_VMA_RESERVE    0x00000100  // Bit 8
#define KEE_VMA_SHARED     0x00000400  // Bit 10
#define KEE_VMA_USEHIGHMEM 0x00000800  // Bit 11
#define KEE_VMA_ALIGN64K   0x00001000  // Bit 12
#define KEE_VMA_USEHMA     0x00002000  // Bit 13

/*ordinal 60*/
APIRET KEEENTRY KernVMAlloc(ULONG size, ULONG flags, PVOID* linaddr,
             PVOID* physaddr, PSHORT sel);

/*ordinal 61*/
APIRET KEEENTRY KernVMFree(PVOID linaddr);

/* this is the lockhandle, like with DevHlp_VMLock */
typedef struct {
   UCHAR lock[12];
} KEEVMLock;

/* this is a page list element, like PageList_s in 16 bit Devhlp */
typedef struct {
   ULONG addr;
   ULONG size;
} KEEVMPageList;

/* the same bits as with DevHlp_VMLock */
#define KEE_VML_NOTBLOCK   0x00000001
#define KEE_VML_CONTIG     0x00000002
#define KEE_VML_16M        0x00000004
#define KEE_VML_WRITEABLE  0x00000008
#define KEE_VML_LONGLOCK   0x00000010
#define KEE_VML_VERIFYONLY 0x00000020
#define KEE_VML_unknown    0x80000000

/*ordinal 62*/
APIRET KEEENTRY KernVMLock(ULONG flags,PVOID linaddr,ULONG size,
            KEEVMLock* lock, KEEVMPageList* pglist,
            PULONG pgsize);

/*ordinal 63*/
APIRET KEEENTRY KernVMUnlock(KEEVMLock* lock);

/*ordinal 64*/
APIRET KEEENTRY KernLinToPageList(PVOID linaddr,ULONG size,KEEVMPageList* list, ULONG *pgcnt);

#define KEE_VMS_UNCOMMIT   0x00000001
#define KEE_VMS_RESIDENT   0x00000002
#define KEE_VMS_SWAPPABLE  0x00000004

/*ordinal 65*/
APIRET KEEENTRY KernVMSetMem(ULONG flags, PVOID linaddr, ULONG size);

/*ordinal 66*/
ULONG KernSelToFlat(ULONG addr16);

/*ordinal 70*/
APIRET KEEENTRY KernDynamicAPI(PVOID addr, ULONG cnt, ULONG dummy, PUSHORT sel);

/*ordinal 80*/
APIRET KEEENTRY KernRASSysTrace(ULONG major,ULONG minor,PVOID buf, ULONG size);

/*ordinal 81*/
APIRET KEEENTRY KernPerfSysTrace(ULONG major,ULONG minor,PVOID buf, ULONG size);

/* this is actually a pointer to the SFT entry for the file */
typedef ULONG KEEhfile;

/*ordinal 100*/
APIRET KEEENTRY KernLockFile(HFILE hfd,KEEhfile* khfd);

/*ordinal 101*/
APIRET KEEENTRY KernUnLockFile(KEEhfile khfd);

/* this is the file size as returned by ordinal 102 */
typedef QWORD KEEfilesize;

/*ordinal 102*/
APIRET KEEENTRY KernGetFileSize(KEEhfile khfd,KEEfilesize* sz);

/*ordinal 103*/
APIRET KEEENTRY KernTestFileCache(KEEhfile khfd);

/*ordinal 104*/
APIRET KEEENTRY KernReadFileAt(KEEhfile khfd,PVOID buf, QWORD off,
                ULONG nbytes, PULONG nread);

typedef struct {
   ULONG pagelistsz;
   KEEVMPageList* pagelist;
   ULONG physlistsz;
   KEEVMPageList* physlist;
} KEECachePages;

/*ordinal 105*/
APIRET KEEENTRY KernReadFileAtCache(KEEhfile khfd,KEECachePages** ptr,
                QWORD off, ULONG nbytes, PULONG nread);

/*ordinal 106*/
APIRET KEEENTRY KernReturnFileCache(KEEhfile khfd,KEECachePages* ptr);

typedef struct {
   ULONG data[8];
} KEEUnicodeStruct;

/*ordinal 120*/
APIRET KEEENTRY KernCreateUconvObject(USHORT codepage, KEEUnicodeStruct* ucs);

/*ordinal 121*/
APIRET KEEENTRY KernStrFromUcs(KEEUnicodeStruct* ucs, PCHAR trgt,
                PCHAR usrc, ULONG trgtsize, ULONG srcsize);
/*ordinal 122*/
APIRET KEEENTRY KernStrToUcs(KEEUnicodeStruct* ucs, PCHAR utrgt, PCHAR src,
              ULONG trgtsize,ULONG srcsize);

#endif
