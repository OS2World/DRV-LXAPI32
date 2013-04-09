/* $Id: init.c,v 1.40 2005/07/11 22:18:33 smilcke Exp $ */

/*
 * lxsh.c
 * Autor:               Stefan Milcke
 * Erstellt am:         14.02.2005
 * Letzte Aenderung am: 05.07.2005
 *
*/

// #include <lxcommon.h>

#define INCL_DOS
#include <os2.h>

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

#include <string.h>

#include <unistd.h>
#include <time.h>
#include <utime.h>
#include <dirent.h>

#define LXISINIT
#include <lxlibc.h>

//----------------------------------- testfn -----------------------------------
void testfn(void)
{
 int rc;
 int h1;
 int h2;
// DevInt3();
 h1=open("/etc/test.log",O_TEXT | O_RDWR,S_IREAD | S_IWRITE);
 if(h1!=-1)
 {
  h2=dup(h1);
  if(h2!=-1)
   close(h2);
  dup2(h1,h2);
  close(h2);
  close(h1);
 }
}

//------------------------------------ main ------------------------------------
int main(int argc,void* argv[])
{
 if(!LXAPI_init_process_started())
 {
  long pid=0;
  // init code here !
  testfn();
  LXAPI_init_process_finished(LXFIN_WAITFORSTABLE|LXFIN_SHOWDRVSIZES|LXFIN_SYNC);
 }
 return 0;
}
