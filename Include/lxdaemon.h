/* $Id: lxdaemon.h,v 1.24 2006/01/05 23:48:20 smilcke Exp $ */

/*
 * lxdaemon.h
 * Autor:               Stefan Milcke
 * Erstellt am:         12.04.2004
 * Letzte Aenderung am: 29.12.2005
 *
*/

#ifndef LXDAEMON_H_INCLUDED
#define LXDAEMON_H_INCLUDED

/* defines for OS/2 include */

/* Standard headers */

/* Application headers */

/* OS/2 Headers */

#include <lxfs.h>

#ifndef LX_MAXPATH
#define LX_MAXPATH  (260)
#endif

// Parameter packet for DAEMON IOCtls
typedef struct _LXIOCPA_DMN_CMDPARMPACKET
{
 // Daemon side managed variables
 ULONG rc;               // Returncode of executed command
 ULONG cbMaxDataLen;     // Maximum size of data packet
 // Driver side managed variables (Don't change this on daemon side!)
 ULONG cmd;              // Command to execute
 ULONG hcmd;             // unique handle
 ULONG cmdBlockID;       // If !=0, the driver Blocks on this ID
 ULONG cbCmdDataLen;     // Size of data needed for command
} LXIOCPA_DMN_CMDPARMPACKET,*PLXIOCPA_DMN_CMDPARMPACKET;

// The data packet is only a void*
// It's storage is managed by the daemon
// It's maximum size is provided by cbMaxDataLen;
// It's real type depends on the command to execute

// Common structs
typedef struct _LXONENAMESTRUCT
{
 char name[LX_MAXPATH];
}LXONENAMESTRUCT,*PLXONENAMESTRUCT;

typedef struct _LXTWONAMESTRUCT
{
 char name1[LX_MAXPATH];
 char name2[LX_MAXPATH];
}LXTWONAMESTRUCT,*PLXTWONAMESTRUCT;

#define LXIOCCAT_DMN       0xF0     // Category for LXAPID Daemon

#define LXIOCFN_DMN_CMDGET                         0x01

// We'll use same ordinals in DOSCALLS as CMD ids
// They can be found in BSEORD.H

#define LXIOCFN_DMN_CMDSETRESULTS                  0x02

// Commands
#define LX_DMN_CMD_SHUTDOWN                        10001

#define LX_DMN_CMD_INC_DATAPACKETLEN               10002

#define LX_DMN_CMD_DEC_DATAPACKETLEN               10003

#define LX_DMN_CMD_SET_LOGNAME                     10004

#define LX_DMN_CMD_LOG                             10005

#define LX_DMN_CMD_DOSQUERYSYSINFO                 10006
typedef struct _LXDOSQUERYSYSINFOSTRUCT
{
 ULONG iStart;
 ULONG iLast;
 ULONG cbBuf;
} LXDOSQUERYSYSINFOSTRUCT, *PLXDOSQUERYSYSINFOSTRUCT;

#define LX_DMN_CMD_RESTART                         10007

#define LX_DMN_CMD_EADELETE                        10008
// Use LXTWONAMESTRUCT

#define LX_DMN_CMD_EAREADSTRING                    10009
typedef struct _LXEAREADSTRINGSTRUCT
{
 char fileName[LX_MAXPATH];
 ULONG eaValLen;
 char eaName[LX_MAXPATH];
 // After this ea value
} LXEAREADSTRINGSTRUCT, *PLXEAREADSTRINGSTRUCT;

#define LX_DMN_CMD_EAWRITESTRING                   10010
typedef struct _LXEAWRITESTRINGSTRUCT
{
 char fileName[LX_MAXPATH];
 ULONG eaValLen;
 char eaName[LX_MAXPATH];
 // After this ea value
} LXEAWRITESTRINGSTRUCT, *PLXEAWRITESTRINGSTRUCT;

#define LX_DMN_CMD_GETFILEINFO                     10011
typedef struct _LXGETFILEINFOSTRUCT
{
 char fileName[LX_MAXPATH];
 char linkName[LX_MAXPATH];
 FILESTATUS4 fs4;
 int num_subdirs;
 int num_files;
 int lc_count;       // Number of links to this file
 int link_broken;    // If 1, the link master was not found
 int mode;
 int uid;
 int gid;
 int symlink;
 int flags;
#define LXGETFILEINFOFLAG_MODE      1
#define LXGETFILEINFOFLAG_UID       2
#define LXGETFILEINFOFLAG_GID       4
#define LXGETFILEINFOFLAG_SYMLINK   8
}LXGETFILEINFOSTRUCT, *PLXGETFILEINFOSTRUCT;

#define LX_DMN_CMD_LXAFSRENAME                     10012
// Use LXTWONAMESTRUCT

/*******************************************************************************/
/* Real DOS wrappers                                                          */
/*******************************************************************************/
#define LX_DMN_CMD_DOSDELETE                       60
// Use LXONENAMESTRUCT

//#define LX_DMN_CMD_DOSCREATEDIR                     66

#define LX_DMN_CMD_DOSMOVE                         67
// Use LXTWONAMESTRUCT

#define LX_DMN_CMD_DOSDELETEDIR                    80
// Use LXONENAMESTRUCT

#define LX_DMN_CMD_DOSSETFILEINFO                  83
typedef struct _LXDOSSETFILEINFOSTRUCT
{
 HFILE hFile;
 ULONG ulInfoLevel;
 FILESTATUS4 filestatus;
 ULONG cbInfoBuf;
} LXDOSSETFILEINFOSTRUCT, *PLXDOSSETFILEINFOSTRUCT;

#define LX_DMN_CMD_DOSSETPATHINFO                  104
typedef struct _LXDOSSETPATHINFOSTRUCT
{
 char pathName[LX_MAXPATH];
 ULONG ulInfoLevel;
 ULONG cbInfoBuf;
 ULONG flOptions;
 union
 {
  FILESTATUS3  fileStatus3;
  EAOP2        eaop2;
 }LXDOSSETPATHINFOUNION;
} LXDOSSETPATHINFOSTRUCT, *PLXDOSSETPATHINFOSTRUCT;

#define LX_DMN_CMD_DOSEXECPGM                      144
typedef struct _LXDOSEXECPGMSTRUCT
{
 char objname[LX_MAXPATH];
 LONG cbObjname;
 ULONG execFlag;
 RESULTCODES res;
 char name[LX_MAXPATH];
 LONG arglen;
 LONG envlen;
} LXDOSEXECPGMSTRUCT,*PLXDOSEXECPGMSTRUCT;

#define LX_DMN_CMD_DOSCOPY                         201
typedef struct _LXDOSCOPYSTRUCT
{
 char sourceFile[LX_MAXPATH];
 char targetFile[LX_MAXPATH];
 ULONG option;
} LXDOSCOPYSTRUCT,*PLXDOSCOPYSTRUCT;

#define LX_DMN_CMD_DOSENUMATTRIBUTE                204
typedef struct _LXDOSENUMATTRIBUTESTRUCT
{
 ULONG ulRefType;
 char vFile[LX_MAXPATH];
 ULONG ulEntry;
 ULONG cbBuf;
 ULONG ulCount;
 ULONG ulInfoLevel;
} LXDOSENUMATTRIBUTESTRUCT,*PLXDOSENUMATTRIBUTESTRUCT;

#define LX_DMN_CMD_DOSQUERYPATHINFO                223
typedef struct _LXDOSQUERYPATHINFOSTRUCT
{
 char pathName[LX_MAXPATH];
 ULONG ulInfoLevel;
 ULONG cbInfoBuf;
 union
 {
  FILESTATUS3  fileStatus3;
  FILESTATUS4  fileStatus4;
  EAOP2        eaop2;
  CHAR         pszName[LX_MAXPATH];
 }LXDOSQUERYPATHINFOUNION;
} LXDOSQUERYPATHINFOSTRUCT,*PLXDOSQUERYPATHINFOSTRUCT;

#define LX_DMN_CMD_DOSSCANENV                      227

#define LX_DMN_CMD_DOSSETFILEPTR                   256
typedef struct _LXDOSSETFILEPTRSTRUCT
{
 HFILE hFile;
 LONG ib;
 ULONG method;
 ULONG ibActual;
} LXDOSSETFILEPTRSTRUCT,*PLXDOSSETFILEPTRSTRUCT;

#define LX_DMN_CMD_DOSCLOSE                        257

#define LX_DMN_CMD_DOSFINDCLOSE                    263
#define LX_DMN_CMD_DOSFINDFIRST                    264
typedef struct _LXDOSFINDFIRSTSTRUCT
{
 HDIR hDir;
 ULONG cbBuf;
 ULONG cFileNames;
 ULONG flAttribute;
 char fileSpec[LX_MAXPATH];
 ULONG ulInfoLevel;
 union
 {
  FILEFINDBUF3 fileFindBuf3;
  FILEFINDBUF4 fileFindBuf4;
  EAOP2        eaop2;
 }LXDOSFINDFIRSTUNION;
} LXDOSFINDFIRSTSTRUCT,*PLXDOSFINDFIRSTSTRUCT;

#define LX_DMN_CMD_DOSFINDNEXT                     265

#define LX_DMN_CMD_DOSCREATEDIR                    270
typedef struct _LXDOSCREATEDIRSTRUCT
{
 char    dirName[LX_MAXPATH];
 EAOP2   eaop2;
}LXDOSCREATEDIRSTRUCT,*PLXDOSCREATEDIRSTRUCT;

#define LX_DMN_CMD_DOSSETFILESIZE                  272
typedef struct _LXDOSSETFILESIZESTRUCT
{
 HFILE hFile;
 ULONG cbSize;
} LXDOSSETFILESIZESTRUCT, *PLXDOSSETFILESIZESTRUCT;

#define LX_DMN_CMD_DOSOPEN                         273
typedef struct _LXDOSOPENSTRUCT
{
 char fileName[LX_MAXPATH];
 HFILE hFile;
 ULONG ulAction;
 ULONG cbFile;
 ULONG ulAttribute;
 ULONG fsOpenFlags;
 ULONG fsOpenMode;
 EAOP2 eaop2;
} LXDOSOPENSTRUCT,*PLXDOSOPENSTRUCT;

#define LX_DMN_CMD_DOSQUERYCURRENTDISK             275
typedef struct _LXDOSQUERYCURRENTDISKSTRUCT
{
 ULONG diskNum;
 ULONG driveMap;
} LXDOSQUERYCURRENTDISKSTRUCT,*PLXDOSQUERYCURRENTDISKSTRUCT;

#define LX_DMN_CMD_DOSQUERYFILEINFO                279
typedef struct _LXDOSQUERYFILEINFOSTRUCT
{
 HFILE hFile;
 ULONG ulInfoLevel;
 FILESTATUS4 filestatus;
 ULONG cbInfoBuf;
} LXDOSQUERYFILEINFOSTRUCT, *PLXDOSQUERYFILEINFOSTRUCT;

#define LX_DMN_CMD_DOSREAD                         281
#define LX_DMN_CMD_DOSWRITE                        282
typedef struct _LXDOSREADWRITESTRUCT
{
 HFILE hFile;
 ULONG cbReadWrite;
 ULONG cbActual;
 char data[1];
} LXDOSREADWRITESTRUCT,*PLXDOSREADWRITESTRUCT;

#define LX_DMN_CMD_DOSBEEP                         286
typedef struct _LXDOSBEEPSTRUCT
{
 ULONG frequency;
 ULONG duration;
} LXDOSBEEPSTRUCT,*PLXDOSBEEPSTRUCT;

/*******************************************************************************/
/* functions                                                                   */
/*******************************************************************************/
extern ULONG LX_DMN_LOG(char* logData);
extern ULONG LX_DMN_SETLOGNAME(char* logFileName);
extern ULONG LX_GETFILEINFO(char* fileName,PLXGETFILEINFOSTRUCT pfs);
extern ULONG LX_LXAFS_RENAME(char* oldName,char* newName);
extern ULONG LX_DOSSCANENV(char* name,char* value);
extern ULONG LX_DOSBEEP(ULONG waitflag
                        ,ULONG frequency,ULONG duration);
extern ULONG LX_DOSOPEN(char* fileName,PHFILE pHf,PULONG pulAction
                        ,ULONG cbFile,ULONG ulAttribute
                        ,ULONG fsOpenFlags,ULONG fsOpenMode
                        ,PEAOP2 peaop2);
extern ULONG LX_DOSFINDFIRST(char* fileSpec,PHDIR pHDir,ULONG flAttribute
                             ,PVOID pfindbuf,ULONG cbBuf,PULONG pcFileNames
                             ,ULONG ulInfoLevel);
extern ULONG LX_DOSCREATEDIR(char* dirName,PEAOP2 peaop2);
extern ULONG LX_DOSFINDNEXT(HDIR hDir,PVOID pfindbuf,ULONG cbBuf,PULONG pcFileNames);
extern ULONG LX_DOSQUERYCURRENTDISK(PULONG pDiskNum,PULONG pDriveMap);
extern ULONG LX_DOSFINDCLOSE(HDIR hDir);
extern ULONG LX_DOSCLOSE(HFILE hFile);
extern ULONG LX_DOSSETFILEPTR(HFILE hFile,LONG ib,ULONG method,PULONG pibActual);
extern ULONG LX_DOSREAD(HFILE hFile,PVOID pBuffer,ULONG cbRead,PULONG pcbActual);
extern ULONG LX_DOSWRITE(HFILE hFile,PVOID pBuffer,ULONG cbWrite,PULONG pcbActual);
extern ULONG LX_DOSQUERYSYSINFO(ULONG iStart,ULONG iLast,VOID* pBuffer,ULONG cbBuf);
extern ULONG LX_DOSEXECPGM(PCHAR pObjname,LONG cbObjname,ULONG execFlag
                           ,PSZ pArg,LONG arglen
                           ,PSZ pEnv,LONG envlen
                           ,PRESULTCODES pRes,PSZ pName);
extern ULONG LX_DOSSETFILESIZE(HFILE hFile,ULONG cbSize);
extern ULONG LX_DOSQUERYFILEINFO(HFILE hFile,ULONG ulInfoLevel
                                 ,VOID* pInfoBuf,ULONG cbInfoBuf);
extern ULONG LX_DOSSETFILEINFO(HFILE hFile,ULONG ulInfoLevel
                               ,VOID* pInfoBuf,ULONG cbInfoBuf);
extern ULONG LX_DOSQUERYPATHINFO(char* name,ULONG ulInfoLevel
                                 ,VOID* pInfoBuf,ULONG cbInfoBuf);
extern ULONG LX_DOSSETPATHINFO(char* name,ULONG ulInfoLevel
                               ,VOID* pInfoBuf,ULONG cbInfoBuf,ULONG flOptions);
extern ULONG LX_DOSENUMATTRIBUTE(ULONG ulRefType,PVOID vFile,ULONG ulEntry
                                 ,PVOID pBuffer,ULONG cbBuf,PULONG pulCount
                                 ,ULONG ulInfoLevel);
extern ULONG LX_DOSDELETE(char* name);
extern ULONG LX_DOSDELETEDIR(char* name);
extern ULONG LX_DOSCOPY(char* sourceFile,char* targetFile,ULONG option);
extern ULONG LX_DOSMOVE(char* oldname,char* newname);


extern ULONG LX_EAREADSTRING(char* fName,char* eaName,void* buffer,PULONG bufLen);
extern ULONG LX_EAWRITESTRING(char* fName,char* eaName,void* buffer,PULONG bufLen);
extern ULONG LX_EADELETE(char* fName,char* eaName);


/*******************************************************************************/
/* Non command functions                                                       */
/*******************************************************************************/
extern volatile unsigned short lx_daemon_pid;
extern unsigned long LX_wait_for_daemon(void);
extern unsigned long LX_daemon_closed(void);

#endif //LXDAEMON_H_INCLUDED
