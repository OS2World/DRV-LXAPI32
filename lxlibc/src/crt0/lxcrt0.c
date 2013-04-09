/* $Id: lxcrt0.c,v 1.5 2005/07/11 22:18:30 smilcke Exp $ */

/*
 * lxcrt0.c
 * Autor:               Stefan Milcke
 * Erstellt am:         09.04.2005
 * Letzte Aenderung am: 05.07.2005
 *
*/

/* Note:
   This module must be compiled with ICC!
 */

extern int LXAPI_main(int argc,void* argv[]);
extern unsigned long LXAPI_init_runtime(void);
extern unsigned long LXAPI_term_runtime(void);

int _System main(int argc,void* argv[])
{
 int rc=-8; // ENOEXEC -> Exec format error
 if(!LXAPI_init_runtime())
 {
  rc=LXAPI_main(argc,argv);
  LXAPI_term_runtime();
 }
 return rc;
}
