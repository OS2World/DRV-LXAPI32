/* $Id: oo_scrio.c,v 1.5 2006/02/16 23:07:21 smilcke Exp $ */

/*
 * oo_scrio.c
 * Autor:               Stefan Milcke
 * Erstellt am:         23.01.2005
 * Letzte Aenderung am: 06.02.2005
 *
*/

#include <lxcommon.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/cpumask.h>
#include <linux/config.h>
#include <linux/sched.h>
#include <asm/processor.h>
#include <asm/i387.h>
#include <asm/uaccess.h>
#include <asm/desc.h>
#include <asm/io.h>
#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/radix-tree.h>

static spinlock_t scr_lock=SPIN_LOCK_UNLOCKED;
static unsigned long phys_scr_addr=0;
static char* virt_scr_addr=0;
static int max_cols=80;
static int max_rows=25;
static int cur_col=0;
static int cur_row=24;
static int cur_attr=LX_SCRATT_GREY;
static int std_attr=LX_SCRATT_GREY;
static int tab_step=8;

static int addr_reg=0x3d4;
static int data_reg=0x3d5;

#define VGA_REG_PAGEADDRHIGH        (12)
#define VGA_REG_PAGEADDRLOW         (13)
#define VGA_REG_CURSORHIGH          (14)
#define VGA_REG_CURSORLOW           (15)

//------------------------------- LX_GET_VGAREG --------------------------------
static inline unsigned char LX_GET_VGAREG(reg)
{
 outb_p(reg,addr_reg);
 return inb_p(data_reg);
}

//--------------------------- _LX_scr_get_cursor_pos ---------------------------
static unsigned long _LX_scr_get_cursor_pos(int* prow,int* pcol)
{
 int row,col,addr;
 addr=(LX_GET_VGAREG(VGA_REG_CURSORHIGH)<<8)
      | (LX_GET_VGAREG(VGA_REG_CURSORLOW));
 row=addr/80;
 col=addr-(row*80);
 *prow=row;
 *pcol=col;
 return 0;
}

//--------------------------- LX_scr_get_cursor_pos ----------------------------
static unsigned long LX_scr_get_cursor_pos(int* prow,int* pcol)
{
 unsigned long f,rc;
 spin_lock_irqsave(&scr_lock,f);
 rc=_LX_scr_get_cursor_pos(prow,pcol);
 spin_unlock_irqrestore(&scr_lock,f);
 return rc;
}

//--------------------------- _LX_scr_set_cursor_pos ---------------------------
static unsigned long _LX_scr_set_cursor_pos(int row,int col)
{
 int addr=row*80+col;
 outb_p(VGA_REG_CURSORHIGH,addr_reg);
 outb_p((addr&0xff00)>>8,data_reg);
 outb_p(VGA_REG_CURSORLOW,addr_reg);
 outb_p(addr&0xff,data_reg);
 return 0;
}

//--------------------------- LX_scr_set_cursor_pos ----------------------------
static unsigned long LX_scr_set_cursor_pos(int row,int col)
{
 unsigned long f,rc;
 spin_lock_irqsave(&scr_lock,f);
 rc=_LX_scr_set_cursor_pos(row,col);
 spin_unlock_irqrestore(&scr_lock,f);
 return rc;
}

//----------------------------- _LX_scr_clear_row ------------------------------
static unsigned long _LX_scr_clear_row(int row)
{
 if(virt_scr_addr)
 {
  int i;
  char* addr=(char*)((unsigned long)virt_scr_addr+row*max_cols*2);
  for(i=0;i<max_cols;i++)
  {
   addr[i*2]=' ';
   addr[i*2+1]=std_attr;
  }
 return 0;
 }
 return -ENXIO;
}

//------------------------------ LX_scr_clear_row ------------------------------
static unsigned long LX_scr_clear_row(int row)
{
 unsigned long f,rc;
 spin_lock_irqsave(&scr_lock,f);
 rc=_LX_scr_clear_row(row);
 spin_unlock_irqrestore(&scr_lock,f);
 return rc;
}

//----------------------------- _LX_scr_scroll_up ------------------------------
static unsigned long _LX_scr_scroll_up(void)
{
 if(virt_scr_addr)
 {
  int r;
  int sz=max_cols*2;
  unsigned long addr=(unsigned long)virt_scr_addr;
  for(r=0;r<max_rows-1;r++)
  {
   memcpy((void*)addr,(void*)addr+sz,sz);
   addr+=sz;
  }
  _LX_scr_clear_row(max_rows-1);
  return 0;
 }
 return -ENXIO;
}

//------------------------------ LX_scr_scroll_up ------------------------------
unsigned long LX_scr_scroll_up(void)
{
 unsigned long f,rc;
 spin_lock_irqsave(&scr_lock,f);
 rc=_LX_scr_scroll_up();
 spin_unlock_irqrestore(&scr_lock,f);
 return rc;
}

//--------------------------- _LX_scr_check_csr_pos ----------------------------
static inline void _LX_scr_check_csr_pos(int* row,int* col)
{
 if(*col>=max_cols)
 {
  *col=0;
  *row=*row+1;
 }
 if(*row>=max_rows)
 {
  _LX_scr_scroll_up();
  *row=max_rows-1;
 }
}

//---------------------------- _LX_scr_put_char_at -----------------------------
static unsigned long _LX_scr_put_char_at(int* row,int* col,char c,char attr,int flags)
{
 if(virt_scr_addr)
 {
  int realrow=(row) ? cur_row : *row;
  int realcol=(col) ? cur_col : *col;
  int index;
  _LX_scr_check_csr_pos(&realrow,&realcol);
  switch(c)
  {
   case '\r':
    realcol=0;
    break;
   case '\n':
    realrow++;
    break;
   case '\t':
    realcol+=tab_step;
    realcol/=tab_step;
    realcol*=tab_step;
    break;
   default:
    index=(realrow*max_cols*2)+(realcol*2);
    if(flags&LX_SCRFLG_SETCHAR)
     virt_scr_addr[index]=(char)c;
    if(flags&LX_SCRFLG_SETATTR)
     virt_scr_addr[index+1]=attr;
    realcol++;
    break;
  }
  _LX_scr_check_csr_pos(&realrow,&realcol);
  if(row)
   *row=realrow;
  else
   cur_row=realrow;
  if(col)
   *col=realcol;
  else
   cur_col=realcol;
  if(!(flags&LX_SCRFLG_NOUPDCSR))
   _LX_scr_set_cursor_pos(cur_row,cur_col);
  return 0;
 }
 return -ENXIO;
}

//----------------------------- LX_scr_put_char_at -----------------------------
unsigned long LX_scr_put_char_at(int* row,int* col,char c,char attr,int flags)
{
 unsigned long f,rc;
 spin_lock_irqsave(&scr_lock,f);
 rc=_LX_scr_put_char_at(row,col,c,attr,flags);
 spin_unlock_irqrestore(&scr_lock,f);
 return rc;
}


//--------------------------- _LX_scr_put_string_at ----------------------------
static unsigned long _LX_scr_put_string_at(int* row,int* col,char* msg)
{
 if(virt_scr_addr)
 {
  char *c=msg;
  int attr=cur_attr;
  int flags=LX_SCRFLG_SETCHAR | LX_SCRFLG_SETATTR | LX_SCRFLG_NOUPDCSR;
  if(strlen(c)>2 && c[0]=='<' && c[2]=='>' && c[1]>='0' && c[1]<='7')
  {
   switch(c[1])
   {
    case '0':
     attr=LX_SCRATT_LIGHT_RED | LX_SCRATT_FLASH;
     break;
    case '1':
     attr=LX_SCRATT_DARK_RED | LX_SCRATT_FLASH;
     break;
    case '2':
     attr=LX_SCRATT_LIGHT_RED;
     break;
    case '3':
     attr=LX_SCRATT_DARK_RED;
     break;
    case '4':
     attr=LX_SCRATT_YELLOW;
     break;
    case '5':
     attr=LX_SCRATT_LIGHT_CYAN;
     break;
    case '6':
     attr=LX_SCRATT_LIGHT_GREEN;
     break;
    case '7':
     attr=LX_SCRATT_LIGHT_BLUE;
     break;
   }
   c++;
   c++;
   c++;
  }
  while(*c)
  {
   _LX_scr_put_char_at(row,col,*c,attr,flags);
   c++;
  }
  _LX_scr_set_cursor_pos(cur_row,cur_col);
  return 0;
 }
 return -ENXIO;
}

//---------------------------- LX_scr_put_string_at ----------------------------
unsigned long LX_scr_put_string_at(int* row,int* col,char* msg)
{
 unsigned long f,rc;
 spin_lock_irqsave(&scr_lock,f);
 rc=_LX_scr_put_string_at(row,col,msg);
 spin_unlock_irqrestore(&scr_lock,f);
 return rc;
}

//----------------------------- LX_scr_put_string ------------------------------
unsigned long LX_scr_put_string(char* msg)
{
 return LX_scr_put_string_at(&cur_row,&cur_col,msg);
}

//------------------------------ _LX_clear_screen ------------------------------
static unsigned long _LX_clear_screen(void)
{
 if(virt_scr_addr)
 {
  int r=0;
  while(r<max_rows)
   LX_scr_clear_row(r);
  return 0;
 }
 return -ENXIO;
}

//------------------------------ LX_clear_screen -------------------------------
unsigned long LX_clear_screen(void)
{
 unsigned long f,rc;
 spin_lock_irqsave(&scr_lock,f);
 rc=_LX_clear_screen();
 spin_unlock_irqrestore(&scr_lock,f);
 return rc;
}

//------------------------------ LX_scr_draw_rect ------------------------------
/*
static inline void LX_scr_draw_rect(char* buffer,char attr,int x1,int y1,int x2,int y2)
{
 int i,index;
 // Horizontal
 for(i=x1;i<=x2;i++)
 {
  index=((y1*max_cols*2)+(i*2));
  buffer[index]=(char)205;
  buffer[index+1]=attr;
  index=((y2*max_cols*2)+(i*2));
  buffer[index]=(char)205;
  buffer[index+1]=attr;
 }
 // Vertical
 for(i=y1;i<=y2;i++)
 {
  index=((i*max_cols*2)+(x1*2));
  if(i==y1)
   buffer[index]=(char)201;
  else if(i==y2)
   buffer[index]=(char)200;
  else
   buffer[index]=(char)186;
  buffer[index+1]=attr;
  index=((i*max_cols*2)+(x2*2));
  if(i==y1)
   buffer[index]=(char)187;
  else if(i==y2)
   buffer[index]=(char)188;
  else
   buffer[index]=(char)186;
  buffer[index+1]=attr;
 }
}
*/

//------------------------------ LX_scr_animation ------------------------------
static void LX_scr_animation(unsigned long direction)
{
/*
 int scrsz=max_cols*2*max_rows;
 char* buffer1=kmalloc(scrsz,GFP_KERNEL);
 char* buffer2=kmalloc(scrsz,GFP_KERNEL);
 if(buffer1 && buffer2)
 {
  int i=0,j=0,x1,x2,y1,y2,index;
  memcpy(buffer1,virt_scr_addr,scrsz);
  memcpy(buffer2,virt_scr_addr,scrsz);
  while(i<max_cols/2 || j<max_rows/2)
  {
   x1=direction ? i : max_cols/2-i-1;
   x2=direction ? max_cols-i : max_cols-(max_cols/2-i);
   y1=direction ? j : max_rows/2-j-1;
   y2=direction ? max_rows-j : max_rows-(max_rows/2-j);
   if(x1<0)
    x1=0;
   if(y1<0)
    y1=0;
   if(x2>=max_cols)
    x2=max_cols-1;
   if(y2>=max_rows)
    y2=max_rows-1;
   LX_scr_draw_rect(buffer1,LX_SCRATT_YELLOW,x1,y1,x2,y2);
   if(i<max_cols/2)
    i++;
   if(!(i&1))
    if(j<max_rows/2)
     j++;
   memcpy(virt_scr_addr,buffer1,scrsz);
   memcpy(buffer1,buffer2,scrsz);
   DevBlock((unsigned long)&scr_lock,10,0);
  }
  memcpy(virt_scr_addr,buffer2,scrsz);
 }
 if(buffer1)
  kfree(buffer1);
 if(buffer2)
  kfree(buffer2);
*/
}

extern int lx_do_verbose;

//------------------------------- LX_open_screen -------------------------------
unsigned long LX_open_screen(void)
{
 unsigned long f;
 int crow,ccol,add;
 spin_lock_irqsave(&scr_lock,f);
 add=(LX_GET_VGAREG(VGA_REG_PAGEADDRHIGH)&0x3f)<<8;
 add|=LX_GET_VGAREG(VGA_REG_PAGEADDRLOW);
 phys_scr_addr=0xb8000;
 if(_LX_scr_get_cursor_pos(&crow,&ccol))
 {
  spin_unlock_irqrestore(&scr_lock,f);
  return -ENXIO;
 }
 cur_row=crow;
 cur_col=ccol;
 _LX_scr_check_csr_pos(&cur_row,&cur_col);
 spin_unlock_irqrestore(&scr_lock,f);
 if(!virt_scr_addr)
  DevVMAlloc(VMDHA_PHYS,4096,(LINEAR)&phys_scr_addr,(LINEAR*)&virt_scr_addr);
 if(!virt_scr_addr)
  return -ENXIO;
 if(lx_do_verbose)
  LX_scr_animation(0);
 return 0;
}

//------------------------------ LX_close_screen -------------------------------
unsigned long LX_close_screen(void)
{
 unsigned long f;
 if(lx_do_verbose)
  LX_scr_animation(1);
 spin_lock_irqsave(&scr_lock,f);
 if(virt_scr_addr)
 {
  DevVMFree((LINEAR)virt_scr_addr);
  virt_scr_addr=0;
 }
 spin_unlock_irqrestore(&scr_lock,f);
 return 0;
}
