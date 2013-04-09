/* $Id: lxfs.h,v 1.6 2005/07/23 21:15:14 smilcke Exp $ */

/*
 * lxfs.h
 * Autor:               Stefan Milcke
 * Erstellt am:         01.03.2005
 * Letzte Aenderung am: 21.07.2005
 *
*/

#ifndef LXFS_H_INCLUDED
#define LXFS_H_INCLUDED

/*******************************************************************************/
/* Constants for lxafs                                                         */
/*******************************************************************************/
// Use short EA names !!
#define LXEAN_LINKBYCOUNT  "LX_LC"
#define LXEAN_LINKEDBY     "LX_LB_%d"
#define LXEAN_LINKEDTO     "LX_LT"
#define LXEAN_MODE         "LX_MODE"
#define LXEAN_UID          "LX_UID"
#define LXEAN_GID          "LX_GID"
#define LXEAN_SYMLINK      "LX_SL"

/*******************************************************************************/
/* high level functions                                                       */
/*******************************************************************************/
struct file;
extern int LX_os2path_from_file(struct file* file,char* buf,int buflen);
extern int LX_os2path_to_lx(const char* os2path,char* lxpath);

#endif //LXFS_H_INCLUDED
