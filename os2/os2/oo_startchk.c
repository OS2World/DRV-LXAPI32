/* $Id: oo_startchk.c,v 1.8 2005/04/24 01:45:22 smilcke Exp $ */

/*
 * oo_startchk.c
 * Autor:               Stefan Milcke
 * Erstellt am:         26.09.2004
 * Letzte Aenderung am: 23.04.2005
 *
*/

#include <lxcommon.h>
#include <linux/kernel.h>
#include <lxapi.h>
#include <lxdaemon.h>

#include <lxsecure.h>

extern int lx_do_last_startup_check;

#define MAX_FLINE_SZ (4096)

//----------------------------- LX_check_log_file ------------------------------
static int LX_check_log_file(char* fileName)
{
 int rc=0;
 unsigned long f;
 static char* txt="OS2: InitComplete reached";
 unsigned long sz;
 char* line=kmalloc(MAX_FLINE_SZ,GFP_KERNEL);
 if(!line)
  return rc;
 if(!(LX_si_open(fileName,&f
                 ,OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS
                 ,OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY)))
 {
  int txtl=strlen(txt);
  rc=1;
  sz=MAX_FLINE_SZ;
  while(LX_si_linein(f,&sz,line))
  {
   sz=MAX_FLINE_SZ;
   if(!strncmp(line,txt,txtl))
   {
    rc=0;
    break;
   }
  }
  LX_si_close(f);
 }
 kfree(line);
 return rc;
}

#define LX_STARTCHK_WAIT   (5)
extern int lx_startup_ok;
//--------------------------- LX_check_last_startup ----------------------------
int LX_check_last_startup(void)
{
 int rc=0;
 char* tmp=kmalloc(LX_MAXPATH,GFP_KERNEL);
 if(!tmp)
  return 0;
 strcpy(tmp,lx_log_path);
 strcat(tmp,"LXAPI32.LO0");
 printk("Checking last startup ...");
 rc=LX_check_log_file(tmp);
 LX_set_sysstate(0,LXSYSSTATE_NOT_INITIALIZED | LXSYSSTATE_NOT_OPERABLE);
 if(rc)
 {
  int i;
  printk(KERN_WARNING " unsuccessfull!");
  if(lx_do_last_startup_check)
  {
   lx_startup_ok=0;
   printk(KERN_EMERG " LXAPI32.SYS not initialized\n");
   LX_set_sysstate(LXSYSSTATE_NOT_OPERABLE,0);
  }
  else
  {
   printk("\n");
   printk(KERN_INFO "Automatic startup check disabled.\n");
   printk(KERN_INFO "If LXAPI32.SYS traps try enabling startup check (remove /NOSTARTCHECK)\n");
   rc=0;
  }
  for(i=0;i<LX_STARTCHK_WAIT;i++)
  {
   sprintf(tmp,KERN_INFO "Pausing for %d second",LX_STARTCHK_WAIT-i);
   if(i<LX_STARTCHK_WAIT-1)
    strcat(tmp,"s");
   strcat(tmp,"  \r");
   LX_scr_put_string(tmp);
   DevBlock((unsigned long)tmp,1000,0);
  }
  LX_scr_put_string("                                         \r");
 }
 else
  printk(" O.K.\n");
 kfree(tmp);
 return rc;
}
