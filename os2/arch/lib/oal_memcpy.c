/* $Id: oal_memcpy.c,v 1.1 2005/05/12 20:01:23 smilcke Exp $ */

/*
 * lal_memcpy.c
 * Autor:               Stefan Milcke
 * Erstellt am:         28.06.2004
 * Letzte Aenderung am: 10.05.2005
 *
*/

#include <lxcommon.h>
#include <linux/config.h>
#include <linux/string.h>

#if defined(TARGET_OS2) && defined(CONFIG_LX_OPTIMIZE_SIZE)
#else

#undef memcpy
#undef memset

void * memcpy(void * to, const void * from, size_t n)
{
#ifdef CONFIG_X86_USE_3DNOW
   return __memcpy3d(to, from, n);
#else
   return __memcpy(to, from, n);
#endif
}

void * memset(void * s, int c, size_t count)
{
   return __memset(s, c, count);
}

#endif
