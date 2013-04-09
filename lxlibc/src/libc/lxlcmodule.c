/* $Id: lxlcmodule.c,v 1.2 2005/07/12 21:59:38 smilcke Exp $ */

/*
 * lxlcmodule.c
 * Autor:               Stefan Milcke
 * Erstellt am:         05.07.2005
 * Letzte Aenderung am: 05.07.2005
 *
*/

#include <lxlibc.h>

#include <linux/unistd.h>

#include <lxlibcsysc.h>

//-------------------------------- init_module ---------------------------------
int init_module(void* file,unsigned long len,__const__ char* args)
{
 return SYSCALL3(__NR_init_module,file,len,args);
}

//------------------------------- delete_module --------------------------------
int delete_module(const char* name,unsigned int flags)
{
 return SYSCALL2(__NR_delete_module,name,flags);
}
