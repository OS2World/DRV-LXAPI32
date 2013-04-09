/* $Id: lxuapioctl.h,v 1.7 2005/07/23 21:15:14 smilcke Exp $ */

/*
 * lxuapioctl.h
 * Autor:               Stefan Milcke
 * Erstellt am:         07.03.2005
 * Letzte Aenderung am: 21.07.2005
 *
*/

#ifndef LXUAPIOCTL_H_INCLUDED
#define LXUAPIOCTL_H_INCLUDED

#include <lxsysstate.h>

#pragma pack(1)

#define LXIOCCAT_UTL       0xA0  // Category for utilities calls
#define LXIOCCAT_SYS       0xA1  // Category for utilities system calls

#define LXIOCFN_UTL_QUERY_SYSTEM_STATE                      0x01
typedef struct _LXIOCPA_UTL_QUERY_SYSTEM_STATE
{
 INT rc;
 ULONG sysstate;
} LXIOCPA_UTL_QUERY_SYSTEM_STATE,*PLXIOCPA_UTL_QUERY_SYSTEM_STATE;

#define LXIOCFN_UTL_INIT_STARTED                            0x02

#define LXIOCFN_UTL_INIT_FINISHED                           0x03
typedef struct _LXIOCPA_UTL_INIT_FINISHED
{
 INT rc;
 ULONG exit_flags;
 ULONG exit_code;
} LXIOCPA_UTL_INIT_FINISHED,*PLXIOCPA_UTL_INIT_FINISHED;
#define LXFIN_SHOWMEMSTATS   1
#define LXFIN_WAITFORSTABLE  2
#define LXFIN_SHOWDRVSIZES   32
#define LXFIN_SYNC           64

/*******************************************************************************/
/* Special fs calls                                                            */
/*******************************************************************************/
#define LXIOCFN_UTL_OS2PATHTOLX                             0x04
typedef struct _LXIOCPA_LFS_OS2PATHTOLX
{
 INT rc;
 char os2path[LX_MAXPATH];
 char lxpath[LX_MAXPATH];
}LXIOCPA_UTL_OS2PATHTOLX,*PLXIOCPA_UTL_OS2PATHTOLX;

/*******************************************************************************/
/* Syscalls                                                                   */
/*******************************************************************************/
#define LXIOCFN_SYS_CALL                                    0x01
typedef struct _LXIOCPA_SYS_CALL
{
 ULONG result;          // OUT
 ULONG sysstate;        // OUT
 ULONG retflags;        // OUT
 ULONG syscallnum;      // IN
 ULONG argc;            // IN
 ULONG arg1;            // IN
 ULONG arg2;            // IN
 ULONG arg3;            // IN
 ULONG arg4;            // IN
 ULONG arg5;            // IN
 ULONG arg6;            // IN
} LXIOCPA_SYS_CALL,*PLXIOCPA_SYS_CALL;
#define LX_SYSCALL_RETFLAG_EXITPROCESS       1

#endif //LXUAPIOCTL_H_INCLUDED
