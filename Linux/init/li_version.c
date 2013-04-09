/* $Id: li_version.c,v 1.3 2005/02/12 19:47:50 smilcke Exp $ */

/*
 * li_version.c
 * Autor:               Stefan Milcke
 * Erstellt am:         30.06.2004
 * Letzte Aenderung am: 11.02.2005
 *
*/

#include <lxcommon.h>

/*
 *  linux/init/version.c
 *
 *  Copyright (C) 1992  Theodore Ts'o
 *
 *  May be freely distributed as part of Linux.
 */

#define UTS_SYSNAME  "LXAPI32"
#define UTS_NODENAME "OS2"
#define UTS_DOMAINNAME "SMILCKE"
#define LINUX_COMPILE_HOST "PCSTM"
#define LINUX_COMPILER "gcc 3.3.5"
#define LINUX_COMPILE_BY "Stefan Milcke"

//#include <linux/compile.h>
#include <linux/module.h>
#include <linux/uts.h>
#include <linux/utsname.h>
#include <linux/version.h>

#define version(a) Version_ ## a
#define version_string(a) version(a)

#define UTS_VERSION "#2.6.6"
#define UTS_MACHINE "OS2"

int version_string(LINUX_VERSION_CODE);

struct new_utsname system_utsname = {
   .sysname = UTS_SYSNAME,
   .nodename   = UTS_NODENAME,
   .release = UTS_RELEASE,
   .version = UTS_VERSION,
   .machine = UTS_MACHINE,
   .domainname = UTS_DOMAINNAME,
};

EXPORT_SYMBOL(system_utsname);

const char *linux_banner =
   "Linux/2 version " UTS_RELEASE " (" LINUX_COMPILE_BY "@"
   LINUX_COMPILE_HOST ") (" LINUX_COMPILER ") " UTS_VERSION "\n";
