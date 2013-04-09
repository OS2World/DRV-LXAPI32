/* $Id: lxlcconvert.c,v 1.1 2005/04/18 22:28:42 smilcke Exp $ */

/*
 * lxlcconvert.c
 * Autor:               Stefan Milcke
 * Erstellt am:         18.04.2005
 * Letzte Aenderung am: 18.04.2005
 *
*/

#define INCL_DOS
#define LXFH_MAP
#include <lxlibc.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>

#define OS_O_RDONLY     0x00000004
#define OS_O_WRONLY     0x00000001
#define OS_O_RDWR       0x00000002
#define OS_O_APPEND     0x00000008
#define OS_O_CREAT      0x00000100
#define OS_O_TRUNC      0x00000200
#define OS_O_EXCL       0x00000400
#define OS_O_TEXT       0x00004000
#define OS_O_BINARY     0x00008000
#define OS_O_NOINHERIT  0x00000080
#define OS_O_RAW        OS_O_BINARY

//------------------------- LXAPI_convert_oflag_to_os --------------------------
int LXAPIENTRY LXAPI_convert_oflag_to_os(int oflag)
{
 int newflag=0;
 switch((oflag&O_ACCMODE))
 {
  case O_RDONLY:
   newflag=OS_O_RDONLY;
   break;
  case O_WRONLY:
   newflag=OS_O_WRONLY;
   break;
  case O_RDWR:
   newflag=OS_O_RDWR;
   break;
 }
 newflag|=(oflag&O_APPEND)    ? OS_O_APPEND     : 0;
 newflag|=(oflag&O_CREAT)     ? OS_O_CREAT      : 0;
 newflag|=(oflag&O_TRUNC)     ? OS_O_TRUNC      : 0;
 newflag|=(oflag&O_EXCL)      ? OS_O_EXCL       : 0;
 newflag|=(oflag&O_TEXT)      ? OS_O_TEXT       : 0;
 newflag|=(oflag&O_BINARY)    ? OS_O_BINARY     : 0;
 newflag|=(oflag&O_NOINHERIT) ? OS_O_NOINHERIT  : 0;
 return newflag;
}

#define OS_S_IREAD      0x0100
#define OS_S_IWRITE     0x0080
#define OS_S_IEXEC      0x0040

//------------------------- LXAPI_convert_omode_to_os --------------------------
int LXAPIENTRY LXAPI_convert_omode_to_os(int omode)
{
 int newmode=0;
 newmode|=(omode&S_IRUSR)     ? OS_S_IREAD   : 0;
 newmode|=(omode&S_IWUSR)     ? OS_S_IWRITE  : 0;
 newmode|=(omode&S_IXUSR)     ? OS_S_IEXEC   : 0;
 return newmode;
}


