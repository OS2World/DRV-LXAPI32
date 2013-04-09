/* $Id: DEVHLP32.H,v 1.15 2005/12/13 22:24:45 smilcke Exp $ */

/*
 * DEVHLP32.h  (for GCC)
 * Autor:               Stefan Milcke
 * Erstellt am:         08.11.2001
 * Letzte Aenderung am: 06.12.2005
 *
*/

#ifndef DEVHLP32_H_INCLUDED
#define DEVHLP32_H_INCLUDED

#include <DEVTYPE.H>

extern void DevHelp(void) asm("DevHlp");
extern void DevHelp_ModifiedDS(void) asm("DevHlp_ModifiedDS");

#if __GNUC__ < 3
#define DEVHLP_ATTR
#else
#define DEVHLP_ATTR     __attribute__((always_inline))
#endif

#define DEVHLP_LINKAGE  static __inline__

/*
 * Check at compile time that something is of a particular type.
 * Always evaluates to 1 so you may use it easily in comparisons.
 */
#define lx_typecheck(type,x) \
({ type __dummy; \
   typeof(x) __dummy2; \
   (void)(&__dummy == &__dummy2); \
   1; \
})

/*******************************************************************************
 * DevInt3
 * Purpose:    Generate an interrupt 3
 * Parameters: None
 * Returns:    Nothing
 *******************************************************************************/
#define DevInt3() __asm__ __volatile__("int $3\n\t": : :"memory")

/*******************************************************************************
 * DevDebugInt3
 * Purpose:    Generate an interrupt 3 if macro LXDEBUG is defined
 * Parameters: None
 * Returns:    Nothing
 *******************************************************************************/
#ifdef LXDEBUG
#define DevDebugInt3 DevInt3
#else
#define DevDebugInt3()
#endif

/*******************************************************************************
 * DevCli
 * Purpose:    Disable interrupts
 * Parameters: None
 * Returns:    Nothing
 *******************************************************************************/
#define DevCli() __asm__ __volatile__("cli\n\t": : :"memory");

/*******************************************************************************
 * DevSti
 * Purpose:    Enable interrupts
 * Parameters: None
 * Returns:    Nothing
 *******************************************************************************/
#define DevSti() __asm__ __volatile__("sti\n\t": : :"memory");

/*******************************************************************************
 * DevCliPushfCli
 * Purpose:    Save flags and disable interrupts
 * Parameters: cpuflags       = storage for cpu flags
 * Returns:    Nothing
 *******************************************************************************/
#define DevPushfCli(cpuflags) \
 do { lx_typecheck(unsigned long,cpuflags); \
      __asm__ __volatile__( \
     "pushfl ; cli ; popl %0":"=g" (cpuflags): /* no input */); } while (0)

/*******************************************************************************
 * DevPopf
 * Purpose:    Restore previously saved cpuflags
 * Parameters: cpuflags       = return value from DevPushfCli
 * Returns:    Nothing
 *******************************************************************************/
#define DevPopf(cpuflags) \
 do { lx_typecheck(unsigned long,cpuflags); \
      __asm__ __volatile__( \
     "testl $0x0200,%0 ; jz 1f ; sti ; 1: ;": /* no output */ :"g" (cpuflags):"memory", "cc"); } while (0)

/*******************************************************************************
 * DevBlock
 * Purpose:    Block the current thread
 * Parameters: id             = Id to be used
 *             msec           = Milliseconds to wait for (-1 = forever)
 *             irqflag        = 0 if block is interruptable, else 1
 * Returns:    0 = Ran, 1 = Timeout, 2 = Ctrl+C
 *******************************************************************************/
#define DHB_RUN                  (0)
#define DHB_TIMEOUT              (1)
#define DHB_INTERRUPTED          (2)

DEVHLP_ATTR DEVHLP_LINKAGE
unsigned char DevBlock(WORD32 id,WORD32 msec,BYTE irqflag)
{
 unsigned char rc;
 __asm__ __volatile__
 (
  "movw  %%ax,%%bx\n\t"
  "shrl  $16,%%eax\n\t"
  "mov   %%di,%%cx\n\t"
  "shrl  $16,%%edi\n\t"
  "shlw  $8,%%dx\n\t"
  "movb  $0x04,%%dl\n\t"
  "call  DevHlp\n\t"
  :"=a"((char)rc)
  :"a"((long) id), "D"((long) msec), "d"((char) irqflag)
  : "ebx"
 );
 return rc;
}

/*******************************************************************************
 * DevRun
 * Purpose:    Run a blocked thread
 * Parameters: id             = Id (used in DevBlock call)
 * Returns:    Count of awakened threads
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevRun(WORD32 id)
{
 short rc;
 __asm__ __volatile__
 (
  "movw  %%ax,%%bx\n\t"
  "shr   $16,%%eax\n\t"
  "movb  $0x05,%%dl\n\t"
  "call  DevHlp\n\t"
  :"=a" ((short)rc)
  :"a"((long) id)
  : "edx", "ebx"
 );
 return rc;
}

/*******************************************************************************
 * DevSegLock
 * Purpose:    Lock a segment down in memory
 * Parameters: Selector       = Selector of memory to lock down
 *             Type           = 0 = short, 1 = long, 3 = long & hi,
 *                              4 = short & verify
 *             NoWaitFlag     = 0 to wait for lock or 1 to return immediately
 *             Handle         = Storage for segment lock handle
 * Returns:    0 = success, 1 = error
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevSegLock(SEL Selector,WORD16 Type,WORD16 NoWaitFlag,HLOCK* Handle)
{
 short rc;
 __asm__ __volatile__
 (
  "push  %%edx\n\t"
  "movb  %%cl,%%bh\n\t"
  "movb  $0x13,%%dl\n\t"
  "call  DevHlp\n\t"
  "setcb %%cl\n\t"
  "xorb  %%ch,%%ch\n\t"
  "pop   %%edx\n\t"
  "mov   %%bx,(%%edx)\n\t"
  "mov   %%ax,2(%%edx)\n\t"
  : "=c" ((short)rc)
  : "a"((short) Selector), "c"((char) Type), "b"((char) NoWaitFlag), "d"((long) Handle)
  : "memory"
 );
 return rc;
}

/*******************************************************************************
 * DevSegUnlock
 * Purpose:    Unlock a segment locked by DevSegLock
 * Parameters: Handle         = Handle of locked segment
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevSegUnlock(HLOCK Handle)
{
 WORD16 rc;
 __asm__ __volatile__
 (
  "movw  %%ax,%%bx\n\t"
  "shrl  $16,%%eax\n\t"
  "movb  $0x14,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorb  %%dh,%%dh\n\t"
  "negw  %%dx\n\t"
  "and   %%dx,%%ax\n\t"
  : "=a" ((short)rc)
  : "a"((long) Handle)
  : "edx", "ebx"
 );
 return rc;
}

/*******************************************************************************
 * DevVMLock
 * Purpose:    Lock a memory region into physical memory
 * Parameters: flags          = flags for the memory object
 *             lin            = Linear address of region to lock
 *             length         = length of memory region
 *             pPageList      = Linear address of an array of page list structs
 *                              to be filled out by the function
 *             pLockHandle    = Linear address for storage of a lock handle
 *             pPageListCount = Linear address for storage of returned page list
 *                              entries
 * Returns:    0 = success, other = error code
 *******************************************************************************/
#define VMDHL_NOBLOCK            (0x0001) // Bit 0
#define VMDHL_CONTIGUOUS         (0x0002) // Bit 1
#define VMDHL_16M                (0x0004) // Bit 2
#define VMDHL_WRITE              (0x0008) // Bit 3
#define VMDHL_LONG               (0x0010) // Bit 4
#define VMDHL_VERIFY             (0x0020) // Bit 5

DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevVMLock(ULONG flags,ULONG lin,ULONG length,
              LINEAR pPageList,LINEAR pLockHandle,LINEAR pPageListCount)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "pushl %%edx\n\t"
  "movb  $0x55,%%dl\n\t"
  "call  DevHlp\n\t"
  "pop   %%ebx\n\t"
  "jc    1f\n\t"
  "movl  %%eax,(%%ebx)\n\t"
  "xorl  %%eax,%%eax\n\t"
  "1:\n\t"
  : "=a" ((long)rc)
  : "a"((long)flags), "b"((long)lin), "c"((long)length),
    "D"((long)pPageList), "S"((long)pLockHandle), "d"((long)pPageListCount)
  : "memory"
 );
 return rc;
}

/*******************************************************************************
 * DevVMAlloc
 * Purpose:    Allocate virtual memory and commit or map given virtual memory
 *             to a given physical address
 * Parameters: flags          = Flags for the memory
 *             size           = Length of memory region
 *             physaddr       = address of physical memory
 *             addr           = address for storage of allocated memory region
 * Returns:    0 = success, other = error code
 *******************************************************************************/
#define VMDHA_16M                (0x0001) // Bit 0
#define VMDHA_FIXED              (0x0002) // Bit 1
#define VMDHA_SWAP               (0x0004) // Bit 2
#define VMDHA_CONTIG             (0x0008) // Bit 3
#define VMDHA_PHYS               (0x0010) // Bit 4
#define VMDHA_PROCESS            (0x0020) // Bit 5
#define VMDHA_SGSCONT            (0x0040) // Bit 6
#define VMDHA_VIRTUAL            (0x0080) // Bit 7
#define VMDHA_GETSEL             (0x0080) // Bit 7 (Alias)
#define VMDHA_RESERVE            (0x0100) // Bit 8
#define VMDHA_SHARED             (0x0400) // Bit 10
#define VMDHA_USEHIGHMEM         (0x0800) // Bit 11
#define VMDHA_ALIGN64K           (0x1000) // Bit 12
#define VMDHA_USEHMA             (0x2000) // Bit 13

DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevVMAlloc(ULONG flags,ULONG size,LINEAR physaddr,LINEAR* addr)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "mov   $0x57,%%dl\n\t"
  "call  DevHlp\n\t"
  "jc    1f\n\t"
  "movl  %%eax,(%%esi)\n\t"
  "xorl  %%eax,%%eax\n\t"
  "1:\n\t"

  : "=a"((long)rc)
  : "a"((long)flags), "c"((long)size), "D"((long)physaddr), "S"((long)addr)
  : "edx", "memory"
 );
  return rc;
}

/*******************************************************************************
 * DevVMFree
 * Purpose:    Frees virtual memory or it's mapping to physical memory
 * Parameters: LinearAddr     = address of memory object
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevVMFree(LINEAR LinearAddr)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  $0x58,%%dl\n\t"
  "call  DevHlp\n\t"
  "jc    1f\n\t"
  "xorl  %%eax,%%eax\n\t"
  "1:\n\t"
  :"=a"((long)rc)
  :"a"((long)LinearAddr)
  :"edx", "memory"
 );
 return rc;
}

/*******************************************************************************
 * DevVMGlobalToProcess
 * Purpose:    Frees virtual memory or it's mapping to physical memory
 * Parameters: flags          = flags of memory object
 *             LinearAddr     = Linear address of memory region
 *             Length         = Length of memory region
 *             ProcessLinAddr = Returned Process linear address
 * Returns:    0 = success, other = error code
 *******************************************************************************/
#define VMDHGP_WRITE             (0x0001)
#define VMDHGP_SELMAP            (0x0002)
#define VMDHGP_SGSCONTROL        (0x0004)
#define VMDHGP_4MEG              (0x0008)

DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevVMGlobalToProcess(ULONG flags,LINEAR LinearAddr,ULONG Length,
                           LINEAR ProcessLinAddr)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  $0x5A,%%dl\n\t"
  "call  DevHlp\n\t"
  "jc    1f\n\t"
  "movl  %%eax,(%%edi)\n\t"
  "xorl  %%eax,%%eax\n\t"
  "1:\n\t"
  :"=a"((long)rc)
  :"a"((long)flags), "b"((long)LinearAddr), "c"((long)Length), "D"((long)ProcessLinAddr)
  : "edx", "memory"
 );
 return rc;
}

/********************************************************************************
 * DevVMProcessToGlobal
 * Purpose:    Frees virtual memory or it's mapping to physical memory
 * Parameters: flags          = flags of memory object
 *             LinearAddr     = Linear address of memory region
 *             Length         = Length of memory region
 *             GlobalLinAddr  = Returned Global linear address
 * Returns:    0 = success, other = error code
 *******************************************************************************/
#define VMDHPG_READONLY          (0x0000)
#define VMDHPG_WRITE             (0x0001)

DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevVMProcessToGlobal(ULONG flags,LINEAR LinearAddr,ULONG Length,
                           LINEAR GlobalLinAddr)
{
  ULONG rc;
  __asm__ __volatile__
  (
   "movb $0x59,%%dl\n\t"
   "call DevHlp\n\t"
   "jc   1f\n\t"
   "movl %%eax,(%%edi)\n\t"
   "xorl %%eax,%%eax\n\t"
   "1:\n\t"
   :"=a"((long)rc)
   :"a"((long)flags), "b"((long)LinearAddr), "c"((long)Length), "D"((long)GlobalLinAddr)
   : "edx", "memory"
  );
  return rc;
}

/*******************************************************************************
 * DevVMSetMem
 * Purpose:    Sets flags for a memory object
 * Parameters: Flags          = flags of memory object
 *             LinearAddr     = Linear address of memory region
 *             Size           = Size of memory region
 * Returns:    0 = success, other = error code
 *******************************************************************************/
#define VMDHS_DECOMMIT           (0x0001)
#define VMDHS_RESIDENT           (0x0002)
#define VMDHS_SWAP               (0x0004)

DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevVMSetMem(ULONG Flags, LINEAR LinearAddr, ULONG Size)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  $0x66,%%dl\n\t"
  "call  DevHlp\n\t"
  :"=a"((long)rc)
  :"a"((long)Flags), "b"((long)LinearAddr), "c"((long)Size)
  :"memory", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevVMUnLock
 * Purpose:    UnLock a memory region into physical memory
 * Parameters: lock           = lock handle
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevVMUnLock(LINEAR lock)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  $0x56,%%dl\n\t"
  "call  DevHlp\n\t"
  "jc    1f\n\t"
  "xorl  %%eax,%%eax\n\t"
  "1:\n\t"
  :"=a"((long)rc)
  :"S"((long)lock)
  :"memory", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevPhysToVirt
 * Purpose:    Convert a physical address to a 16:16 address
 * Parameters: PhysAddr       = Physical address
 *             Length         = Length of memory region
 *             VirtAddr       = Virtual address
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevPhysToVirt(PHYSICAL PhysAddr,WORD16 Length, VIRTUAL* VirtAddr)
{
 WORD16 rc;
 __asm__ __volatile__
 (
  "movw  %%ax,%%bx\n\t"
  "shrw  $16,%%eax\n\t"
  "xorb  %%dh,%%dh\n\t"
  "movw  $0x115,%%dx\n\t"  // Note: 115 for moving result to es:di
  "call  DevHlp\n\t"
  "jc    1f\n\t"
  "movw  %%di,(%%esi)\n\t"
  "movw  %%es,2(%%esi)\n\t"
  "xorl  %%eax,%%eax\n\t"
  "1:\n\t"
  :"=a"((short)rc)
  :"a"((long) PhysAddr), "c"((long)Length), "S"((long)VirtAddr)
  : "ebx", "edi"
 );
 return rc;
}

/*******************************************************************************
 * DevVirtToPhys
 * Purpose:    Convert a locked 16:16 address to physical
 * Parameters: VirtAddr       = Virtual address to convert (must be locked)
 *             PhysAddr       = Storage for physical address
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevVirtToPhys(VIRTUAL VirtAddr,PHYSICAL *PhysAddr)
{
 WORD16 rc;
 __asm__ __volatile__
 (
  "pushl %%ecx\n\t"
  "pushw %%ds\n\t"
  "movw  %%ax,%%si\n\t"
  "shrl  $16,%%eax\n\t"
  "movw  %%ax,%%ds\n\t"
  "movb  $0x16,%%dl\n\t"
  "call  DevHlp_ModifiedDS\n\t"
  "popw  %%ds\n\t"
  "popl  %%ecx\n\t"
  "jc    1f\n\t"
  "movw  %%bx,(%%ecx)\n\t"
  "movw  %%ax,2(%%ecx)\n\t"
  "xorl  %%eax,%%eax\n\t"
  "1:\n\t"
  :"=a"((short)rc)
  :"a"((long)VirtAddr), "c"((long)PhysAddr)
  :"edx", "ebx", "esi", "memory"
 );
 return rc;
}

/*******************************************************************************
 * DevPhysToUVirt
 * Purpose:    Convert a physical address to an LDT address
 * Parameters: PhysAddr       = Physical address to convert
 *             Length         = Length of virtual segment
 *             Type           = Type (0 = code, 1 = data, 2 = free, 5 = video)
 *             Tag            = Tag (only with type 5)
 *             VirtAddr       = Storage for virtual address
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevPhysToUVirt(PHYSICAL PhysAddr,WORD16 Length,BYTE Type,BYTE Tag,
                      VIRTUAL* VirtAddr)
{
 WORD16 rc;
 __asm__ __volatile__
 (
  "pushw %%es\n\t"
  "pushl %%edi\n\t"
  "movw  %%ax,%%bx\n\t"
  "shrl  $16,%%eax\n\t"
  "movb  %%dl,%%dh\n\t"
  "movb  $0x17,%%dl\n\t"
  "call  DevHlp\n\t"
  "pop   %%edi\n\t"
  "jc    1f\n\t"
  "movw  %%bx,(%%edi)\n\t"
  "movw  %%es,2(%%edi)\n\t"
  "xorl  %%eax,%%eax\n\t"
  "1:\n\t"
  "popw  %%es\n\t"
  :"=a"((short)rc)
  :"a"((long)PhysAddr), "c"((short)Length)
   ,"d"((char)Type),"S"((short)Tag),"D"((long)VirtAddr)
  : "ebx", "memory"
 );
 return rc;
}

/*******************************************************************************
 * DevPhysAlloc
 * Purpose:    Allocate a block of physical memory
 * Parameters: Size           = Size of memory to allocate
 *             LoFlag         = Flag: 0 = above 1MB, 1 = below 1MB
 *             PhysAddr       = Storage for address of memory block
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevPhysAlloc(WORD32 Size,BYTE LoFlag,PHYSICAL* PhysAddr)
{
 WORD16 rc;
 __asm__ __volatile__
 (
  "movw  %%ax,%%bx\n\t"
  "shrl  $16,%%eax\n\t"
  "movb  %%dl,%%dh\n\t"
  "movb  $0x18,%%dl\n\t"
  "call  DevHlp\n\t"
  "jc    1f\n\t"
  "mov   %%bx,(%%esi)\n\t"
  "mov   %%ax,2(%%esi)\n\t"
  "xorl  %%eax,%%eax\n\t"
  "1:\n\t"
  :"=a"((short)rc)
  :"a"((long)Size), "d"((char)LoFlag), "S"((long)PhysAddr)
  : "ebx", "memory"
 );
 return rc;
}

/*******************************************************************************
 * DevPhysFree
 * Purpose:    Free a block of physical memory
 * Parameters: PhysAddr       = Address of memory block to be freed
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevPhysFree(PHYSICAL PhysAddr)
{
 WORD16 rc;
 __asm__ __volatile__
 (
  "movw  %%ax,%%bx\n\t"
  "shrl  $16,%%eax\n\t"
  "movb  $0x19,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorb  %%dh,%%dh\n\t"
  : "=d"((short)rc)
  : "a"((long)PhysAddr)
  : "ebx"
 );
 return rc;
}

/*******************************************************************************
 * DevIRQSet
 * Purpose:    Register an interrupt handler for an IRQ
 * Parameters: IRQ            = IRQ Number to handle
 *             Handler        = CS:Address of IRQ handler
 *             SharableFlag   = Sharable: 1 = sharable, 0 = exclusive
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevIRQSet(WORD16 IRQ,FUNCTION Handler,BYTE SharableFlag)
{
 WORD16 rc;
 __asm__ __volatile__
 (
  "movb  %%dl,%%dh\n\t"
  "movb  $0x1b,%%dl\n\t"
  "call  DevHlp\n\t"
  "jc    1f\n\t"
  "xorw  %%ax,%%ax\n\t"
  "1:\n\t"
  :"=a"((short)rc)
  :"b"((long)IRQ), "a"((long)Handler), "d"((char)SharableFlag)
  :"memory"
 );
 return rc;
}

/*******************************************************************************
 * DevIRQClear
 * Purpose:    Remove a registered IRQ handler
 * Parameters: IRQ            = IRQ Number to release
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevIRQClear(WORD16 IRQ)
{
 WORD16 rc;
 __asm__ __volatile__
 (
  "movb  $0x1c,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorb  %%dh,%%dh\n\t"
  :"=d"((short)rc)
  :"b"((short)IRQ)
  :"memory"
 );
 return rc;
}

/*******************************************************************************
 * DevAttachDD
 * Purpose:    Attach to another device driver for IDC
 * Parameters: DriverName     = Device driver name (from device header)
 *             IDCData        = Storage for IDC data
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevAttachDD(WORD32 DriverName,WORD32 IDCData)
{
 WORD16 rc;
 __asm__ __volatile__
 (
  "movb  $0x2a,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorb  %%dh,%%dh\n\t"
  :"=d"((short)rc)
  :"b"((long)DriverName), "D"((long)IDCData)
  :"memory"
 );
 return rc;
}

/*******************************************************************************
 * DevAllocGDTSel
 * Purpose:    Attach to another device driver for IDC
 * Parameters: SelectorArray  = Storage for allocated GDT selectors (16:16)
 *             Count          = Number of GDT selectors to allocate
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevAllocGDTSel(WORD32 SelectorArray,WORD16 Count)
{
 WORD16 rc;
 __asm__ __volatile__
 (
  "pushw %%es\n\t"
  "mov   %%edi,%%eax\n\t"
  "shrl  $16,%%eax\n\t"
  "movw  %%ax,%%es\n\t"
  "movb  $0x2d,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorb  %%dh,%%dh\n\t"
  "negw  %%dx\n\t"
  "andw  %%dx,%%ax\n\t"
  "popw  %%es\n\t"
  :"=a"((short)rc)
  :"D"((long)SelectorArray), "c"((short)Count)
  :"memory", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevPhysToGDTSelector
 * Purpose:    Convert a 32 bit address to a GDT selector
 * Parameters: Address        = 32 bit physical address to convert
 *             Length         = Length of fabricated segment
 *             Selector       = Selector to refer to 32 bit address
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevPhysToGDTSelector(PHYSICAL Address,WORD16 Length,SEL Selector)
{
 WORD16 rc;
 __asm__ __volatile__
 (
  "movw  %%ax,%%bx\n\t"
  "shrl  $16,%%eax\n\t"
  "movb  $0x2e,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorb  %%dh,%%dh\n\t"
  "negw  %%dx\n\t"
  "andw  %%dx,%%ax\n\t"
  :"=a"((short)rc)
  :"a"((long)Address), "c"((short)Length), "S"((short)Selector)
  :"memory", "edx", "ebx"
 );
 return rc;
}

/*******************************************************************************
 * DevEOI
 * Purpose:    Signal end of interrupt processing to PIC
 * Parameters: IRQ            = IRQ Number to end
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
VOID DevEOI(WORD16 IRQ)
{
 __asm__ __volatile__
 (
  "movb  $0x31,%%dl\n\t"
  "call  DevHlp\n\t"
  :
  :"a"((short)IRQ)
  :"memory", "edx"
 );
}

/*******************************************************************************
 * DevRegisterPDD
 * Purpose:    Register driver for PDD-VDD communications
 * Parameters: DriverName     = Device driver name (from device header)
 *             IDCData        = Storage for IDC data
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevRegisterPDD(WORD32 DriverName,WORD32 EntryPoint)
{
 WORD16 rc;
 __asm__ __volatile__
 (
  "pushw %%ds\n\t"
  "pushw %%es\n\t"
  "movw  %%ax,%%si\n\t"
  "shrl  $16,%%eax\n\t"
  "movw  %%ax,%%ds\n\t"
  "movw  %%bx,%%di\n\t"
  "shrl  $16,%%ebx\n\t"
  "movw  %%bx,%%es\n\t"
  "movb  $0x50,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorb  %%dh,%%dh\n\t"
  "popw  %%es\n\t"
  "popw  %%ds\n\t"
  :"=d"((short)rc)
  :"a"((long)DriverName), "b"((long)EntryPoint)
  :"memory", "esi", "edi"
 );
 return rc;
}

/*******************************************************************************
 * DevBeep
 * Purpose:    Generate a beep
 * Parameters: Frequency      = Beep pitch in hz
 *             Duration       = Beep duration
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevBeep(WORD16 Frequency,WORD16 Duration)
{
 WORD16 rc;
 __asm__ __volatile__
 (
  "movb  $0x52,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorb  %%dh,%%dh\n\t"
  "negw  %%dx\n\t"
  "andw  %%dx,%%ax\n\t"
  :"=a"((short)rc)
  :"b"((short)Frequency), "c"((short)Duration)
  :"memory", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevFreeGDTSel
 * Purpose:    Free an allocated GDT selector at task time
 * Parameters: Selector       = Selector to free
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevFreeGDTSel(SEL Selector)
{
 WORD16 rc;
 __asm__ __volatile__
 (
  "movb  $0x53,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorb  %%dh,%%dh\n\t"
  "negw  %%dx\n\t"
  "andw  %%dx,%%ax\n\t"
  :"=a"((short)rc)
  :"a"((short)Selector)
  :"memory", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevPhysToGDTSel
 * Purpose:    Map a physical address to a GDT selector
 * Parameters: Address        = 32 bit physical address to convert
 *             Length         = Length of fabricated segment
 *             Selector       = Selector to refer to 32 bit address
 *             Access         = Descriptor access privileges
 * Returns:    0 = success, other = error code
 *******************************************************************************/
#define GDTSEL_R3CODE            (0x0000)
#define GDTSEL_R3DATA            (0x0001)
#define GDTSEL_R2CODE            (0x0003)
#define GDTSEL_R2DATA            (0x0004)
#define GDTSEL_R0CODE            (0x0005)
#define GDTSEL_R0DATA            (0x0006)
#define GDTSEL_ADDR32            (0x0080)
#define GDTSEL_R2CODE32          (GDTSEL_R2CODE|GDTSEL_ADDR32)
#define GDTSEL_R0CODE32          (GDTSEL_R0CODE|GDTSEL_ADDR32)

DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevPhysToGDTSel(PHYSICAL Address,WORD32 Length,SEL Selector,BYTE Access)
{
 WORD16 rc;
 __asm__ __volatile__
 (
  "movb  %%dl,%%dh\n\t"
  "andl  $0x0000ffff,%%ecx\n\t"
  "movb  $0x54,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorb  %%dh,%%dh\n\t"
  "negw  %%dx\n\t"
  "andw  %%dx,%%ax\n\t"
  :"=a"((short)rc)
  :"a"((long)Address), "c"((long)Length), "S"((short)Selector), "d"((char)Access)
  :"memory"
 );
 return rc;
}

/*******************************************************************************
 * DevVirtToLin
 * Purpose:    Convert a virtual address to linear
 * Parameters: Selector       = Selector to convert
 *             Offset         = Offset to convert
 *             Linear         = Storage for linear address
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevVirtToLin(SEL Selector,WORD32 Offset,LINEAR* Linear)
{
 WORD16 rc;
 __asm__ __volatile__
 (
  "movb  $0x5b,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "movl  %%eax,(%%edi)\n\t"
  "xorb  %%dh,%%dh\n\t"
  "negw  %%dx\n\t"
  "andw  %%dx,%%ax\n\t"
  :"=a"((short)rc)
  :"a"((short)Selector), "S"((long)Offset), "D"((long)Linear)
  :"memory", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevVerifyAccess
 * Purpose:    Verify that the user process has correct access rights
 * Parameters: Addr           = address
 *             Length         = Length of memory area
 *             AccType        = Access type (0 = read, 1 = read/write)
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
WORD16 DevVerifyAccess(ULONG Addr,WORD16 Length,BYTE AccType)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  %%dl,%%dh\n\t"
  "xorl  %%edi,%%edi\n\t"
  "movw  %%ax,%%di\n\t"
  "shrl  $16,%%eax\n\t"
  "movb  $0x27,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorl  %%eax,%%eax\n\t"
  "movb  %%dl,%%al\n\t"
  :"=a"((long)rc)
  :"a"((long)Addr), "c"((short)Length), "d"((char)AccType)
  :"memory", "edi"
 );
 return rc;
}

/*******************************************************************************
 * DevResetEventSem
 * Purpose:    Reset an event semaphore
 * Parameters: hEvent         = Semaphore handle
 *             pNumPosts      = Number of posts
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevResetEventSem(ULONG hEvent,FARPTR16 pNumPosts)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  $0x6a,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorl  %%eax,%%eax\n\t"
  "movb  %%dl,%%al\n\t"
  :"=a"((long)rc)
  :"a"((long)hEvent), "D"((long)pNumPosts)
  :"memory", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevCloseEventSem
 * Purpose:    Close an event semaphore
 * Parameters: hEvent         = Semaphore handle
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevCloseEventSem(ULONG hEvent)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  $0x68,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorl  %%eax,%%eax\n\t"
  "movb  %%dl,%%al\n\t"
  :"=a"((long)rc)
  :"a"((long)hEvent)
  :"memory", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevOpenEventSem
 * Purpose:    Open an event semaphore
 * Parameters: hEvent         = Semaphore handle
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevOpenEventSem(ULONG hEvent)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  $0x67,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorl  %%eax,%%eax\n\t"
  "movb  %%dl,%%al\n\t"
  :"=a"((long)rc)
  :"a"((long)hEvent)
  :"memory", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevPostEventSem
 * Purpose:    Post an event semaphore
 * Parameters: hEvent         = Semaphore handle
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevPostEventSem(ULONG hEvent)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  $0x69,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorl  %%eax,%%eax\n\t"
  "movb  %%dl,%%al\n\t"
  :"=a"((long)rc)
  :"a"((long)hEvent)
  :"memory", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevFreeCtxHook
 * Purpose:    Frees a context hook
 * Parameters: HookHandle     = Hook handle
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevFreeCtxHook(ULONG HookHandle)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  $0x64,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorl  %%eax,%%eax\n\t"
  "movb  %%dl,%%al\n\t"
  :"=a"((long)rc)
  :"a"((long)HookHandle)
  :"memory", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevAllocateCtxHook
 * Purpose:    Allocates a context hook
 * Parameters: HookHandler    = Offset of context hook handler
 *             HookHandle     = Storage for context hook handle
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevAllocateCtxHook(ULONG HookHandler,LINEAR HookHandle)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movl  $-1,%%ebx\n\t"
  "movb  $0x63,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "movl  %%eax,(%%edi)\n\t"
  "xorl  %%eax,%%eax\n\t"
  "movb  %%dl,%%al\n\t"
  :"=a"((long)rc)
  :"a"((long)HookHandler),"D"((long)HookHandle)
  :"memory", "edx", "ebx"
 );
 return rc;
}

/*******************************************************************************
 * DevArmCtxHook
 * Purpose:    Arm a context hook
 * Parameters: HookData       = Data to be passed on to the hook handler
 *             HookHandle     = Context hook handle
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevArmCtxHook(ULONG HookData,ULONG HookHandle)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movl  $-1,%%ecx\n\t"
  "movb  $0x65,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorl  %%eax,%%eax\n\t"
  "movb  %%dl,%%al\n\t"
  :"=a"((long)rc)
  :"a"((long)HookData),"b"((long)HookHandle)
  :"memory", "edx", "ecx"
 );
 return rc;
}

/*******************************************************************************
 * DevPageListToLin
 * Purpose:    map physical memory pages to a linear address
 * Parameters: Size           = Count of bytes to be mapped
 *             pPageList      = Flat pointer to array of page list entries
 *             LinearAddr     = Linear address
 * Returns:    0 = success, other = error code
 *******************************************************************************/
typedef struct
{
 DWORD physaddr;
 DWORD size;
} PAGELIST;
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevPageListToLin(ULONG Size,PAGELIST* pPageList,LINEAR* LinearAddr)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  $0x5f,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "movl  %%eax,(%%esi)\n\t"
  "xorl  %%eax,%%eax\n\t"
  "movb  %%dl,%%al\n\t"
  :"=a"((long)rc)
  :"c"((long)Size),"D"((long)pPageList),"S"((long)LinearAddr)
  :"memory", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevLinToPageList
 * Purpose:    Translate a linear address range to an array of page list
 * Parameters: LinearAddr     = Starting linear address
 *             Size           = Size of the range to translate (<64KB)
 *             pPageList      = Flat pointer to array of page list entries
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevLinToPageList(LINEAR LinearAddr,ULONG Size,PAGELIST* pPageList)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  $0x5e,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorl  %%eax,%%eax\n\t"
  "movb  %%dl,%%al\n\t"
  :"=a"((long)rc)
  :"a"((long)LinearAddr),"c"((long)Size),"D"((long)pPageList)
  :"memory", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevLinToGDTSelector
 * Purpose:    Converts a linear address to a virtual address
 * Parameters: Selector       = Selector to map
 *             LinearAddr     = Linear address
 *             Size           = Size of the range to convert (<64KB)
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevLinToGDTSelector(SEL Selector,LINEAR LinearAddr,ULONG Size)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  $0x5c,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorl  %%eax,%%eax\n\t"
  "movb  %%dl,%%al\n\t"
  :"=a"((long)rc)
  :"a"((short)Selector),"b"((long)LinearAddr),"c"((long)Size)
  :"memory", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevGetDOSVar
 * Purpose:    Return address of a kernel variable
 * Parameters: VarNumber      = Index wanted
 *             VarMember      = Only for index 14 and 16
 *             KernelVar      = Storage for kernel variable
 * Returns:    0 = success, other = error code
 *******************************************************************************/
#define DHGETDOSV_SYSINFOSEG        (1)
#define DHGETDOSV_LOCINFOSEG        (2)
#define DHGETDOSV_VECTORSDF         (4)
#define DHGETDOSV_VECTORREBOOT      (5)
#define DHGETDOSV_YIELDFLAG         (7)
#define DHGETDOSV_TCYIELDFLAG       (8)
#define DHGETDOSV_DOSTABLE          (9)
#define DHGETDOSV_DOSCODEPAGE       (11)
#define DHGETDOSV_INTERRUPTLEV      (13)
#define DHGETDOSV_DEVICECLASSTABLE  (14)
#define DHGETDOSV_DMQSSELECTOR      (15)
#define DHGETDOSV_APMINFO           (16)
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevGetDOSVar(ULONG VarNumber,ULONG VarMember,VOID* KernelVar)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "xorl  %%ebx,%%ebx\n\t"
  "movb  $0x24,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "movw  %%bx,(%%edi)\n\t"
  "movw  %%ax,2(%%edi)\n\t"
  "xorl  %%eax,%%eax\n\t"
  "movb  %%dl,%%al\n\t"
  :"=a"((long)rc)
  :"a"((long)VarNumber) ,"c"((long)VarMember),"D"((long)KernelVar)
  :"memory", "edx", "ebx"
 );
 return rc;
}

/*******************************************************************************
 * DevSetTimer
 * Purpose:    Add a timer handler to the list of timer handlers
 * Parameters: TimerHandler   = Address of timer handler
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevSetTimer(DWORD TimerHandler)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  $0x1d,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorl  %%eax,%%eax\n\t"
  "movb  %%dl,%%al\n\t"
  :"=a"((long)rc)
  :"a"((long)TimerHandler)
  :"memory", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevTickCount
 * Purpose:    Register new timer handler or modify existing handler
 * Parameters: TimerHandler   = Address of timer handler
 *             TickCount      = Value of "ticks" (0-ffff)
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevTickCount(DWORD TimerHandler,DWORD TickCount)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  $0x33,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorl  %%eax,%%eax\n\t"
  "movb  %%dl,%%al\n\t"
  :"=a"((long)rc)
  :"a"((long)TimerHandler), "b"((long)TickCount)
  :"memory", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevYield
 * Purpose:    Yield the CPU to a scheduled thread
 * Parameters: None
 * Returns:    Nothing
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
VOID DevYield(VOID)
{
 __asm__ __volatile__
 (
  "movb  $0x02,%%dl\n\t"
  "call  DevHlp\n\t"
  :
  :
  :"memory", "ebx", "esi", "edi"
 );
}

/*******************************************************************************
 * DevTCYield
 * Purpose:    Yield the CPU to a scheduled time critical thread
 * Parameters: None
 * Returns:    Nothing
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
VOID DevTCYield(VOID)
{
 __asm__ __volatile__
 (
  "movb  $0x03,%%dl\n\t"
  "call  DevHlp\n\t"
  :
  :
  :"memory", "ebx", "esi", "edi"
 );
}

/*******************************************************************************
 * DevCheckYield
 * Purpose:    Checks if yield is neccessary
 * Parameters: None
 * Returns:    Nothing
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevCheckYield(VOID)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  $0x07,%%al\n\t"
  "movb  $0x24,%%dl\n\t"
  "call  DevHlp\n\t"
  "movw  %%ax,%%es\n\t"
  "xorl  %%eax,%%eax\n\t"
  "movb  %%al,%%es:(%%bx)\n\t"
  :"=a"((long)rc)
  :
  :"memory"

 );
 return rc;
}

/*******************************************************************************
 * DevSecurity
 * Purpose:
 * Parameters:
 * Returns:    0 = success, other = error code
 *******************************************************************************/
#define DHSEC_GETEXPORT          (0x048a78df8)
#define DHSEC_SETIMPORT          (0x073ae3627)
#define DHSEC_GETINFO            (0x033528882)
#define SEC_EXPORT_MAJOR_VERSION (0x01)
#define SEC_EXPORT_MINOR_VERSION (0x00)
#define SEC_IMPORT_MAJOR_VERSION (0x01)
#define SEC_IMPORT_MINOR_VERSION (0x00)

DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevSecurity(ULONG func,VOID* ptr)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movb  $0x44,%%dl\n\t"
  "call  DevHlp\n\t"
  "setc  %%dl\n\t"
  "xorl  %%eax,%%eax\n\t"
  "movb  %%dl,%%al\n\t"
  :"=a"((long)rc)
  :"a"((long)func), "c"((long)ptr)
  :"memory", "ebx", "edx", "esi", "edi"
 );
 return rc;
}

/*******************************************************************************
 * DevCreateSpinLock
 * Purpose:    Create a spinlock for SMP (6f or 79? Not clear)
 * Parameters: pLockHandle    = Storage for lock handle
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
ULONG DevCreateSpinLock(ULONG* pLockHandle)
{
 ULONG rc;
 __asm__ __volatile__
 (
  "movw  %%ax,%%bx\n\t"
  "shrl  $16,%%eax\n\t"
  "movb  0x6f,%%dl\n\t"
  "call  DevHlp\n\t"
  :"=a"((long)rc)
  :"a"((long)pLockHandle)
  :"memory", "ebx", "edx"
 );
 return rc;
}

/*******************************************************************************
 * DevAcquireSpinLock
 * Purpose:    Acquire a spinlock for SMP
 * Parameters: lockHandle     = Handle for spin lock
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
VOID DevAcquireSpinLock(ULONG lockHandle)
{
 __asm__ __volatile__
 (
  "movw  %%ax,%%bx\n\t"
  "shrl  $16,%%eax\n\t"
  "movb  0x71,%%dl\n\t"
  "call  DevHlp\n\t"
  :
  :"a"((long)lockHandle)
  :"memory", "ebx", "edx"
 );
}

/*******************************************************************************
 * DevReleaseSpinLock
 * Purpose:    Release a spinlock for SMP
 * Parameters: lockHandle     = Handle for spin lock
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
VOID DevReleaseSpinLock(ULONG lockHandle)
{
 __asm__ __volatile__
 (
  "movw  %%ax,%%bx\n\t"
  "shrl  $16,%%eax\n\t"
  "movb  0x72,%%dl\n\t"
  "call  DevHlp\n\t"
  :
  :"a"((long)lockHandle)
  :"memory", "ebx", "edx"
 );
}

/*******************************************************************************
 * DevFreeSpinLock
 * Purpose:    Frees a spinlock for SMP
 * Parameters: lockHandle     = Handle for spin lock
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
VOID DevFreeSpinLock(ULONG lockHandle)
{
 __asm__ __volatile__
 (
  "movw  %%ax,%%bx\n\t"
  "shrl  $16,%%eax\n\t"
  "movb  0x7a,%%dl\n\t"
  "call  DevHlp\n\t"
  :
  :"a"((long)lockHandle)
  :"memory", "ebx", "edx"
 );
}

/*******************************************************************************
 * DevRequestFastSem
 * Purpose:    Requests a fast ram semaphore
 * Parameters: lockptr        = Pointer to semaphore
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
int DevRequestFastSem(volatile unsigned int* lockptr)
{
 int rc;
 __asm__ __volatile__
 (
  "xorl  %%eax,%%eax\n\t"
  "lock\n\t"
  "xchg  (%%ecx),%%eax\n\t"
  : "=a" ((int)rc)
  : "c" ((long)lockptr)
  : "memory", "eax"
 );
 return rc;
}

/*******************************************************************************
 * DevReleaseFastSem
 * Purpose:    Releases a fast ram semaphore
 * Parameters: lockptr        = Pointer to semaphore
 * Returns:    0 = success, other = error code
 *******************************************************************************/
DEVHLP_ATTR DEVHLP_LINKAGE
int DevReleaseFastSem(volatile unsigned int* lockptr)
{
 int rc;
 __asm__ __volatile__
 (
  "movl  $1,%%eax\n\t"
  "lock\n\t"
  "xchg  (%%ecx),%%eax\n\t"
  : "=a" ((int)rc)
  : "c" ((long)lockptr)
  : "memory", "eax"
 );
 return rc;
}


#endif // DEVHLP32_H_INCLUDED
