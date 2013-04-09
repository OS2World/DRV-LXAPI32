/* $Id: lxdispatch.c,v 1.25 2004/09/29 23:27:14 smilcke Exp $ */

/*
 * lxdispatch.c
 * Autor:               Stefan Milcke
 * Erstellt am:         12.11.2001
 * Letzte Aenderung am: 26.09.2004
 *
*/

#include <lxcommon.h>

#include <lxapi.h>
#include <lxapioctl.h>

void run_timer_list(void);

//--------------------------------- LX_EoiIrq ----------------------------------
void LX_EoiIrq(int irq)
{
 DevEOI(irq);
}

extern FUNCTION TimerHandler_PTR;
extern FUNCTION IRQHandler16_0_PTR;
extern FUNCTION IRQHandler16_1_PTR;
extern FUNCTION IRQHandler16_2_PTR;
extern FUNCTION IRQHandler16_3_PTR;
extern FUNCTION IRQHandler16_4_PTR;
extern FUNCTION IRQHandler16_5_PTR;
extern FUNCTION IRQHandler16_6_PTR;
extern FUNCTION IRQHandler16_7_PTR;
extern FUNCTION IRQHandler16_8_PTR;
extern FUNCTION IRQHandler16_9_PTR;
extern FUNCTION IRQHandler16_10_PTR;
extern FUNCTION IRQHandler16_11_PTR;
extern FUNCTION IRQHandler16_12_PTR;
extern FUNCTION IRQHandler16_13_PTR;
extern FUNCTION IRQHandler16_14_PTR;
extern FUNCTION IRQHandler16_15_PTR;

static FUNCTION *IRQHandlerArray[]=
{
 &IRQHandler16_0_PTR,
 &IRQHandler16_1_PTR,
 &IRQHandler16_2_PTR,
 &IRQHandler16_3_PTR,
 &IRQHandler16_4_PTR,
 &IRQHandler16_5_PTR,
 &IRQHandler16_6_PTR,
 &IRQHandler16_7_PTR,
 &IRQHandler16_8_PTR,
 &IRQHandler16_9_PTR,
 &IRQHandler16_10_PTR,
 &IRQHandler16_11_PTR,
 &IRQHandler16_12_PTR,
 &IRQHandler16_13_PTR,
 &IRQHandler16_14_PTR,
 &IRQHandler16_15_PTR,
};

struct lx_irq_state
{
 atomic_t use_count;
 atomic_t handled;
};

static struct lx_irq_state irq_states[16]={{{0}}};

//------------------------------ LX_timer_started ------------------------------
int LX_timer_started(void)
{
 return atomic_read(&(irq_states[0].use_count));
}

//----------------------------- LX_set_irq_handled -----------------------------
void LX_set_irq_handled(int irq)
{
 if(irq>=0 && irq<=15)
 {
  atomic_set(&(irq_states[irq].handled),1);
  if(0!=irq)
   DevEOI(irq);
 }
}

//------------------------------- LX_irq_handled -------------------------------
int LX_irq_handled(int irq)
{
 if(irq>0 && irq<=15)
  return atomic_read(&(irq_states[irq].handled));
 else
  return 0;
}

//------------------------------- LX_RequestIrq --------------------------------
int LX_RequestIrq(int irq,int flags)
{
 int rc=0;
 if(irq>=0 && irq<=15)
 {
  if(0==atomic_read(&(irq_states[irq].use_count)))
  {
   if(0==irq)
    rc=DevSetTimer((unsigned long)TimerHandler_PTR);
   else
    rc=DevIRQSet(irq,*(IRQHandlerArray[irq]),flags);
  }
  if(!rc)
   atomic_inc(&(irq_states[irq].use_count));
 }
 else
  return -1;
 return rc;
}

//--------------------------------- LX_FreeIrq ---------------------------------
void LX_FreeIrq(int irq)
{
 if(irq>0 && irq<=15)
 {
  if(atomic_dec_and_test(&(irq_states[irq].use_count)))
   DevIRQClear(irq);
 }
 else if(0==irq)
  atomic_set(&(irq_states[irq].use_count),1);
}
