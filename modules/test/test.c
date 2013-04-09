/* $Id: test.c,v 1.2 2005/07/25 20:04:23 smilcke Exp $ */

/*
 * test.c
 * Autor:               Stefan Milcke
 * Erstellt am:         17.07.2005
 * Letzte Aenderung am: 24.07.2005
 *
*/

#include <lxcommon.h>

extern void* module;

//----------------------------------- initFn -----------------------------------
void initFn(void)
{
 void* m;
 m=module;
 if(m)
  m=0;
}

//----------------------------------- exitFn -----------------------------------
void exitFn(void)
{
}
