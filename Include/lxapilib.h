/* $Id: lxapilib.h,v 1.17 2004/05/27 20:33:34 smilcke Exp $ */

/*
 * lxapilib.h
 * Autor:               Stefan Milcke
 * Erstellt am:         08.01.2002
 * Letzte Aenderung am: 07.04.2004
 *
*/

#ifndef LXAPILIB_H_INCLUDED
#define LXAPILIB_H_INCLUDED

#include <linux/module.h>

// Returns 0, if successfull, any other value indicates a failure
int LX_initlxapi(void);

#pragma pack(1)
struct lx_enum_module
{
 char name[32];
 unsigned long versioncode;
 int active;
};

struct lx_enum_video_device
{
 char name[32];
 char devname[16];
 unsigned long type;
 unsigned long hardware;
};
#pragma pack()

#endif //LXAPILIB_H_INCLUDED
