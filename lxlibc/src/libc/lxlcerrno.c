/* $Id: lxlcerrno.c,v 1.1 2005/04/18 22:28:42 smilcke Exp $ */

/*
 * lxlcerrno.c
 * Autor:               Stefan Milcke
 * Erstellt am:         18.04.2005
 * Letzte Aenderung am: 18.04.2005
 *
*/

#define INCL_DOS
#define LXFH_MAP
#include <lxlibc.h>

//----------------------------------- _errno -----------------------------------
int* _errno(void)
{
 int* rc;
 __asm__ __volatile__
 (
  "pushl %%ecx\n\t"
  "pushl %%edx\n\t"
  "call  _errno\n\t"
  "popl  %%edx\n\t"
  "popl  %%ecx\n\t"
  :"=a"((unsigned long)rc)
 );
 return rc;
}
