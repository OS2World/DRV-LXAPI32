/* $Id: lxerrmap.c,v 1.2 2005/06/11 22:52:23 smilcke Exp $ */

/*
 * lxerrmap.c
 * Autor:               Stefan Milcke
 * Erstellt am:         09.06.2005
 * Letzte Aenderung am: 10.06.2005
 *
*/

#define INCL_DOS
#define INCL_DOSERRORS
//#define INCL_DOSSEMAPHORES
#include <os2.h>

#include <linux/errno.h>

static int errormap[][2]=
{
 {NO_ERROR,    0},
 {ERROR_INVALID_FUNCTION,        -EINTR},
 {ERROR_FILE_NOT_FOUND,          -ENOENT},
 {ERROR_PATH_NOT_FOUND,          -ENOENT},
 {ERROR_TOO_MANY_OPEN_FILES,     -EMFILE},
 {ERROR_ACCESS_DENIED,           -EACCES},
 {ERROR_INVALID_HANDLE,          -EBADF},
 {ERROR_NOT_ENOUGH_MEMORY,       -ENOMEM},
 {ERROR_INVALID_ACCESS,          -EACCES},
 {ERROR_INVALID_DATA,            -EINVAL},
 {ERROR_INVALID_DRIVE,           -EIO},
 {ERROR_CURRENT_DIRECTORY,       -ENOENT},
};

#define MAPSIZE (sizeof(errormap)/sizeof(errormap[0]))

//----------------------------- lxmap_dosapi_error -----------------------------
int lxmap_dosapi_error(int err)
{
 int rc=-1,index;
 for(index=0;index<MAPSIZE && errormap[index][0]!=err;index++);
 if(index<MAPSIZE)
  rc=errormap[index][1];
 else
  rc=-err;
 return rc;
}
