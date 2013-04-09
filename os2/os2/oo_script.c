/* $Id: oo_script.c,v 1.13 2006/01/05 23:48:25 smilcke Exp $ */

/*
 * lx_script.c
 * Autor:               Stefan Milcke
 * Erstellt am:         06.07.2004
 * Letzte Aenderung am: 29.12.2005
 *
*/

#include <lxcommon.h>
#define INCL_DOSFILEMGR
#include <lxcommon.h>
#include <devrp.h>

#include <lxapilib.h>
#include <lxapi.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/setup.h>

#include <lxdaemon.h>

#include <lxapi.h>

#include <lxsecure.h>

extern char lx_drv_homepath[];

LIST_HEAD(lx_options_list);

//-------------------------- LX_add_or_replace_option --------------------------
void LX_add_or_replace_option(struct lx_module_option* pop)
{
 struct list_head* lh;
 struct lx_module_option* p;
 list_for_each(lh,&lx_options_list)
 {
  p=list_entry(lh,struct lx_module_option,list);
  if(!strcmp(pop->module_name,p->module_name)
     && !strcmp(pop->option_name,p->option_name))
  {
   strcpy(p->option_val,pop->option_val);
   kfree(pop);
  }
 }
 list_add(&pop->list,&lx_options_list);
}

//------------------------- LX_set_option_from_string --------------------------
void LX_set_option_from_string(char* op)
{
 char* mod_name;
 char* pIn;
 char* pOut;
 mod_name=kmalloc(MODULE_NAME_LEN,GFP_KERNEL);
 if(mod_name)
 {
  pIn=op;
  pOut=mod_name;
  // First get module name
  while(*pIn && *pIn!=' ')
   *pOut++=*pIn++;
  *pOut=0;
  if(*pIn)
  {
   pIn++;
   // Now loop over all options and add a lx_module_option
   while(*pIn)
   {
    struct lx_module_option* pop=kmalloc(sizeof(struct lx_module_option),GFP_KERNEL);
    if(!pop)
     break;
    memset(pop,0,sizeof(struct lx_module_option));
    strcpy(pop->module_name,mod_name);
    pOut=pop->option_name;
    while(*pIn && *pIn!='=' && *pIn!=' ')
     *pOut++=*pIn++;
    *pOut=0;
    if(!*pIn || !*(pIn++))
    {
     printk("Error in line %s\n",op);
     kfree(pop);
     kfree(mod_name);
     return;
    }
    pOut=pop->option_val;
    while(*pIn && *pIn!=' ')
     *pOut++=*pIn++;
    *pOut=0;
    LX_add_or_replace_option(pop);
    pIn++;
   }
  }
 }
 kfree(mod_name);
}

char saved_command_line[COMMAND_LINE_SIZE];

//--------------------------------- LX_vmlinuz ---------------------------------
void LX_vmlinuz(char* cmd)
{
 strncpy(saved_command_line,cmd,COMMAND_LINE_SIZE);
}

extern void fastcall LXA_kernel_starter(void);
//---------------------------------- LX_init -----------------------------------
void LX_init(char* cmd)
{
 if(!(lx_sysstate&LXSYSSTATE_KERNEL_BOOT_STARTED))
 {
  unsigned long tid=0;
  LXA_CreateThread((unsigned long)LXA_kernel_starter,&tid);
 }
}

static struct _lx_scr_commands
{
 char* cmdName;
 void(*fn)(char* cmd);
} lx_scr_commands[]=
{
 {
  .cmdName  ="options ",
  .fn       =&LX_set_option_from_string,
 },
 {
  .cmdName  ="vmlinuz ",
  .fn       =&LX_vmlinuz,
 },
 {
  .cmdName  ="init ",
  .fn       =&LX_init,
 }
};
const unsigned int lx_num_scr_commands=(sizeof(lx_scr_commands)/sizeof(struct _lx_scr_commands));
#define MAX_SCR_LINE_LEN   (1000)

//------------------------- LX_build_full_script_name --------------------------
static char* LX_build_full_script_name(char* scrname)
{
 char* full_scr_name=kmalloc(LX_MAXPATH,GFP_KERNEL);
 if(full_scr_name)
 {
  if(strlen(lx_root_path)==0)
  {
   if(LX_DOSSCANENV("LXAPIROOT",lx_root_path)
      || strlen(lx_root_path)==0)
    strcpy(lx_root_path,lx_drv_homepath);
   if(lx_root_path[strlen(lx_root_path)-1]!='\\')
    strcat(lx_root_path,"\\");
  }
  sprintf(full_scr_name,"%s%s",lx_root_path,scrname);
 }
 return full_scr_name;
}

//----------------------------- LX_process_script ------------------------------
void LX_process_script(char* scrname)
{
 unsigned long f;
 char* full_scr_name;
 char* line;
 int i;
 unsigned long sz;
 int tmp;
 full_scr_name=LX_build_full_script_name(scrname);
 line=kmalloc(MAX_SCR_LINE_LEN,GFP_KERNEL);
 if(!(full_scr_name) || !(line))
  goto out;
 if(!(LX_si_open(full_scr_name,&f
                 ,OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS
                 ,OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY)))
 {
  int lc=0;
  printk("executing script %s\n",full_scr_name);
  sz=MAX_SCR_LINE_LEN;
  while(LX_si_linein(f,&sz,line))
  {
   sz=MAX_SCR_LINE_LEN;
   lc++;
   tmp=strlen(line);
   if(tmp && (line[tmp-1]==0x0d || line[tmp-1]==0x0a))
    line[--tmp]=(char)0;
   if(tmp && line[0]!='#')
   {
    for(i=0;i<lx_num_scr_commands;i++)
    {
     if(!strncmp(line,lx_scr_commands[i].cmdName
                 ,strlen(lx_scr_commands[i].cmdName)-1))
     {
      if(line[strlen(lx_scr_commands[i].cmdName)-1]==' ')
      {
       lx_scr_commands[i].fn(&(line[strlen(lx_scr_commands[i].cmdName)]));
       break;
      }
      else if(line[strlen(lx_scr_commands[i].cmdName)-1]==(char)0)
      {
       lx_scr_commands[i].fn(NULL);
       break;
      }
     }
     if(i>=lx_num_scr_commands)
      printk("Error in %s in line %d\n",full_scr_name,lc);
    }
   }
  }
  LX_si_close(f);
  printk("finished script %s\n",full_scr_name);
 }
out:
 if(line)
  kfree(line);
 if(full_scr_name)
  kfree(full_scr_name);
}
