/* $Id: ok_printk.c,v 1.6 2006/01/05 23:48:24 smilcke Exp $ */

/*
 * ok_printk.c
 * Autor:               Stefan Milcke
 * Erstellt am:         21.03.2004
 * Letzte Aenderung am: 30.12.2005
 *
*/

#include <lxcommon.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/smp_lock.h>
#include <linux/console.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>        /* For in_interrupt() */
#include <linux/config.h>
#include <linux/delay.h>
#include <linux/smp.h>
#include <linux/security.h>
#include <linux/bootmem.h>

#include <asm/uaccess.h>

#include <lxdaemon.h>
extern void LX_Verbose(const char* str);

#define __LOG_BUF_LEN   (1 << CONFIG_LOG_BUF_SHIFT)

/* printk's without a loglevel use this.. */
#define DEFAULT_MESSAGE_LOGLEVEL 4 /* KERN_WARNING */

/* We show everything that is MORE important than this.. */
#define MINIMUM_CONSOLE_LOGLEVEL 1 /* Minimum loglevel we let people use */
#define DEFAULT_CONSOLE_LOGLEVEL 7 /* anything MORE serious than KERN_DEBUG */

DECLARE_WAIT_QUEUE_HEAD(log_wait);

int console_printk[4] = {
   DEFAULT_CONSOLE_LOGLEVEL,  /* console_loglevel */
   DEFAULT_MESSAGE_LOGLEVEL,  /* default_message_loglevel */
   MINIMUM_CONSOLE_LOGLEVEL,  /* minimum_console_loglevel */
   DEFAULT_CONSOLE_LOGLEVEL,  /* default_console_loglevel */
};

EXPORT_SYMBOL(console_printk);

/*
 * Array of consoles built from command line options (console=)
 */
struct console_cmdline
{
   char  name[8];       /* Name of the driver       */
   int   index;            /* Minor dev. to use     */
   char  *options;         /* Options for the driver   */
};

#define MAX_CMDLINECONSOLES 8

static struct console_cmdline console_cmdline[MAX_CMDLINECONSOLES];
static int preferred_console = -1;

int oops_in_progress;

/*
 * console_sem protects the console_drivers list, and also
 * provides serialisation for access to the entire console
 * driver system.
 */
static DECLARE_MUTEX(console_sem);
struct console *console_drivers;
/*
 * This is used for debugging the mess that is the VT code by
 * keeping track if we have the console semaphore held. It's
 * definitely not the perfect debug tool (we don't know if _WE_
 * hold it are racing, but it helps tracking those weird code
 * path in the console code where we end up in places I want
 * locked without the console sempahore held
 */
static int console_locked;

/*
 * logbuf_lock protects log_buf, log_start, log_end, con_start and logged_chars
 * It is also used in interesting ways to provide interlocking in
 * release_console_sem().
 */
static spinlock_t logbuf_lock = SPIN_LOCK_UNLOCKED;

static char __log_buf[__LOG_BUF_LEN];
static char *log_buf = __log_buf;
static int log_buf_len = __LOG_BUF_LEN;

#define LOG_BUF_MASK (log_buf_len-1)
#define LOG_BUF(idx) (log_buf[(idx) & LOG_BUF_MASK])

/*
 * The indices into log_buf are not constrained to log_buf_len - they
 * must be masked before subscripting
 */
static unsigned long log_start;  /* Index into log_buf: next char to be read by syslog() */
static unsigned long con_start;  /* Index into log_buf: next char to be sent to consoles */
static unsigned long log_end; /* Index into log_buf: most-recently-written-char + 1 */
static unsigned long logged_chars; /* Number of chars produced since last read+clear operation */

/* Flag: console code may call schedule() */
static int console_may_schedule;

/*
 * Call the console drivers on a range of log_buf
 */
static void __call_console_drivers(unsigned long start, unsigned long end)
{
   struct console *con;

   for (con = console_drivers; con; con = con->next) {
      if ((con->flags & CON_ENABLED) && con->write)
         con->write(con, &LOG_BUF(start), end - start);
   }
}

/*
 * Write out chars from start to end - 1 inclusive
 */
static void _call_console_drivers(unsigned long start,
            unsigned long end, int msg_log_level)
{
   if (msg_log_level < console_loglevel &&
         console_drivers && start != end) {
      if ((start & LOG_BUF_MASK) > (end & LOG_BUF_MASK)) {
         /* wrapped write */
         __call_console_drivers(start & LOG_BUF_MASK,
                  log_buf_len);
         __call_console_drivers(0, end & LOG_BUF_MASK);
      } else {
         __call_console_drivers(start, end);
      }
   }
}

/*
 * Call the console drivers, asking them to write out
 * log_buf[start] to log_buf[end - 1].
 * The console_sem must be held.
 */
static void call_console_drivers(unsigned long start, unsigned long end)
{
   unsigned long cur_index, start_print;
   static int msg_level = -1;

   if (((long)(start - end)) > 0)
      BUG();

   cur_index = start;
   start_print = start;
   while (cur_index != end) {
      if (  msg_level < 0 &&
         ((end - cur_index) > 2) &&
         LOG_BUF(cur_index + 0) == '<' &&
         LOG_BUF(cur_index + 1) >= '0' &&
         LOG_BUF(cur_index + 1) <= '7' &&
         LOG_BUF(cur_index + 2) == '>')
      {
         msg_level = LOG_BUF(cur_index + 1) - '0';
         cur_index += 3;
         start_print = cur_index;
      }
      while (cur_index != end) {
         char c = LOG_BUF(cur_index);
         cur_index++;

         if (c == '\n') {
            if (msg_level < 0) {
               /*
                * printk() has already given us loglevel tags in
                * the buffer.  This code is here in case the
                * log buffer has wrapped right round and scribbled
                * on those tags
                */
               msg_level = default_message_loglevel;
            }
            _call_console_drivers(start_print, cur_index, msg_level);
            msg_level = -1;
            start_print = cur_index;
            break;
         }
      }
   }
   _call_console_drivers(start_print, end, msg_level);
}

/**
 * acquire_console_sem - lock the console system for exclusive use.
 *
 * Acquires a semaphore which guarantees that the caller has
 * exclusive access to the console system and the console_drivers list.
 *
 * Can sleep, returns nothing.
 */
void acquire_console_sem(void)
{
   if (in_interrupt())
      BUG();
   down(&console_sem);
   console_locked = 1;
   console_may_schedule = 1;
}
EXPORT_SYMBOL(acquire_console_sem);

int is_console_locked(void)
{
   return console_locked;
}
EXPORT_SYMBOL(is_console_locked);

/**
 * release_console_sem - unlock the console system
 *
 * Releases the semaphore which the caller holds on the console system
 * and the console driver list.
 *
 * While the semaphore was held, console output may have been buffered
 * by printk().  If this is the case, release_console_sem() emits
 * the output prior to releasing the semaphore.
 *
 * If there is output waiting for klogd, we wake it up.
 *
 * release_console_sem() may be called from any context.
 */
void release_console_sem(void)
{
   unsigned long flags;
   unsigned long _con_start, _log_end;
   unsigned long wake_klogd = 0;

   for ( ; ; ) {
      spin_lock_irqsave(&logbuf_lock, flags);
      wake_klogd |= log_start - log_end;
      if (con_start == log_end)
         break;         /* Nothing to print */
      _con_start = con_start;
      _log_end = log_end;
      con_start = log_end;    /* Flush */
      spin_unlock_irqrestore(&logbuf_lock, flags);
      call_console_drivers(_con_start, _log_end);
   }
   console_locked = 0;
   console_may_schedule = 0;
   up(&console_sem);
   spin_unlock_irqrestore(&logbuf_lock, flags);
   if (wake_klogd && !oops_in_progress && waitqueue_active(&log_wait))
      wake_up_interruptible(&log_wait);
}
EXPORT_SYMBOL(release_console_sem);

/** console_conditional_schedule - yield the CPU if required
 *
 * If the console code is currently allowed to sleep, and
 * if this CPU should yield the CPU to another task, do
 * so here.
 *
 * Must be called within acquire_console_sem().
 */
void console_conditional_schedule(void)
{
   if (console_may_schedule && need_resched()) {
      set_current_state(TASK_RUNNING);
      schedule();
   }
}
EXPORT_SYMBOL(console_conditional_schedule);

void console_print(const char *s)
{
   printk(KERN_EMERG "%s", s);
}
EXPORT_SYMBOL(console_print);

void console_unblank(void)
{
   struct console *c;

   /*
    * Try to get the console semaphore. If someone else owns it
    * we have to return without unblanking because console_unblank
    * may be called in interrupt context.
    */
   if (down_trylock(&console_sem) != 0)
      return;
   console_locked = 1;
   console_may_schedule = 0;
   for (c = console_drivers; c != NULL; c = c->next)
      if ((c->flags & CON_ENABLED) && c->unblank)
         c->unblank();
   release_console_sem();
}
EXPORT_SYMBOL(console_unblank);

/*
 * The console driver calls this routine during kernel initialization
 * to register the console printing procedure with printk() and to
 * print any messages that were printed by the kernel before the
 * console driver was initialized.
 */
void register_console(struct console * console)
{
   int     i;
   unsigned long flags;

   /*
    * See if we want to use this console driver. If we
    * didn't select a console we take the first one
    * that registers here.
    */
   if (preferred_console < 0) {
      if (console->index < 0)
         console->index = 0;
      if (console->setup == NULL ||
          console->setup(console, NULL) == 0) {
         console->flags |= CON_ENABLED | CON_CONSDEV;
         preferred_console = 0;
      }
   }

   /*
    * See if this console matches one we selected on
    * the command line.
    */
   for(i = 0; i < MAX_CMDLINECONSOLES && console_cmdline[i].name[0]; i++) {
      if (strcmp(console_cmdline[i].name, console->name) != 0)
         continue;
      if (console->index >= 0 &&
          console->index != console_cmdline[i].index)
         continue;
      if (console->index < 0)
         console->index = console_cmdline[i].index;
      if (console->setup &&
          console->setup(console, console_cmdline[i].options) != 0)
         break;
      console->flags |= CON_ENABLED;
      console->index = console_cmdline[i].index;
      if (i == preferred_console)
         console->flags |= CON_CONSDEV;
      break;
   }

   if (!(console->flags & CON_ENABLED))
      return;

   /*
    * Put this console in the list - keep the
    * preferred driver at the head of the list.
    */
   acquire_console_sem();
   if ((console->flags & CON_CONSDEV) || console_drivers == NULL) {
      console->next = console_drivers;
      console_drivers = console;
   } else {
      console->next = console_drivers->next;
      console_drivers->next = console;
   }
   if (console->flags & CON_PRINTBUFFER) {
      /*
       * release_console_sem() will print out the buffered messages
       * for us.
       */
      spin_lock_irqsave(&logbuf_lock, flags);
      con_start = log_start;
      spin_unlock_irqrestore(&logbuf_lock, flags);
   }
   release_console_sem();
}
EXPORT_SYMBOL(register_console);

int unregister_console(struct console * console)
{
        struct console *a,*b;
   int res = 1;

   acquire_console_sem();
   if (console_drivers == console) {
      console_drivers=console->next;
      res = 0;
   } else {
      for (a=console_drivers->next, b=console_drivers ;
           a; b=a, a=b->next) {
         if (a == console) {
            b->next = a->next;
            res = 0;
            break;
         }
      }
   }

   /* If last console is removed, we re-enable picking the first
    * one that gets registered. Without that, pmac early boot console
    * would prevent fbcon from taking over.
    */
   if (console_drivers == NULL)
      preferred_console = -1;


   release_console_sem();
   return res;
}
EXPORT_SYMBOL(unregister_console);

#define MAX_IRQ_PRINTK_BUF (1024)
#define MAX_PRINTK_BUF     (1024)
//----------------------------------- printk -----------------------------------
asmlinkage int printk(const char* fmt, ...)
{
 va_list args;
 unsigned long flags;
 int printed_len;
 static char printk_buf[MAX_PRINTK_BUF]="";
 static char irq_printk_buf[MAX_IRQ_PRINTK_BUF]="";
 static char irq_tmp_buf[MAX_PRINTK_BUF];
 if(atomic_read(&lx_in_ISR))
 { // In interrupt we fill in irq_printk_buf. It will be printed at task time
   // on the next printk call
  spin_lock_irqsave(&logbuf_lock,flags);
  va_start(args,fmt);
  printed_len=vscnprintf(irq_tmp_buf,sizeof(irq_tmp_buf),fmt,args);
  va_end(args);
  if(strlen(irq_printk_buf)+strlen(irq_tmp_buf)<MAX_IRQ_PRINTK_BUF-1)
   strcat(irq_printk_buf,irq_tmp_buf);
  spin_unlock_irqrestore(&logbuf_lock,flags);
 }
 else
 {
  // Wait until lock is unlocked, because LX_Verbose or LX_write_log may
  // enable IRQ
  while((char)0!=printk_buf[0])
   DevBlock((unsigned long)printk_buf,10,0);
  // Now it is save to print
  // Check, if an interrupt has called printk and print it
  while((char)0!=irq_printk_buf[0])
  {
   spin_lock_irqsave(&logbuf_lock,flags);
   strncpy(printk_buf,irq_printk_buf,MAX_PRINTK_BUF);
   irq_printk_buf[0]=(char)0;
   spin_unlock_irqrestore(&logbuf_lock,flags);
   if(255==(*P_INIT_COUNT))
    LX_Verbose(printk_buf);
   LX_write_log(printk_buf);
  }
  // Now print given string
  spin_lock_irqsave(&logbuf_lock,flags);
  va_start(args,fmt);
  printed_len=vscnprintf(printk_buf,sizeof(printk_buf),fmt,args);
  va_end(args);
  spin_unlock_irqrestore(&logbuf_lock,flags);
  if(255==(*P_INIT_COUNT))
   LX_Verbose(printk_buf);
  LX_write_log(printk_buf);
  printk_buf[0]=(char)0;
 }
 return 0;
}

/*
 * printk rate limiting, lifted from the networking subsystem.
 *
 * This enforces a rate limit: not more than one kernel message
 * every printk_ratelimit_jiffies to make a denial-of-service
 * attack impossible.
 */
int __printk_ratelimit(int ratelimit_jiffies, int ratelimit_burst)
{
   static spinlock_t ratelimit_lock = SPIN_LOCK_UNLOCKED;
   static unsigned long toks = 10*5*HZ;
   static unsigned long last_msg;
   static int missed;
   unsigned long flags;
   unsigned long now = jiffies;

   spin_lock_irqsave(&ratelimit_lock, flags);
   toks += now - last_msg;
   last_msg = now;
   if (toks > (ratelimit_burst * ratelimit_jiffies))
      toks = ratelimit_burst * ratelimit_jiffies;
   if (toks >= ratelimit_jiffies) {
      int lost = missed;
      missed = 0;
      toks -= ratelimit_jiffies;
      spin_unlock_irqrestore(&ratelimit_lock, flags);
      if (lost)
         printk(KERN_WARNING "printk: %d messages suppressed.\n", lost);
      return 1;
   }
   missed++;
   spin_unlock_irqrestore(&ratelimit_lock, flags);
   return 0;
}
EXPORT_SYMBOL(__printk_ratelimit);

/* minimum time in jiffies between messages */
int printk_ratelimit_jiffies = 5*HZ;

/* number of messages we send before ratelimiting */
int printk_ratelimit_burst = 10;

int printk_ratelimit(void)
{
   return __printk_ratelimit(printk_ratelimit_jiffies,
            printk_ratelimit_burst);
}
EXPORT_SYMBOL(printk_ratelimit);

/*
 * The indices into log_buf are not constrained to log_buf_len - they
 * must be masked before subscripting
 */
static unsigned long log_start;  /* Index into log_buf: next char to be read by syslog() */
static unsigned long con_start;  /* Index into log_buf: next char to be sent to consoles */
static unsigned long log_end; /* Index into log_buf: most-recently-written-char + 1 */
static unsigned long logged_chars; /* Number of chars produced since last read+clear operation */

/*
 * Commands to do_syslog:
 *
 *    0 -- Close the log.  Currently a NOP.
 *    1 -- Open the log. Currently a NOP.
 *    2 -- Read from the log.
 *    3 -- Read all messages remaining in the ring buffer.
 *    4 -- Read and clear all messages remaining in the ring buffer
 *    5 -- Clear ring buffer.
 *    6 -- Disable printk's to console
 *    7 -- Enable printk's to console
 * 8 -- Set level of messages printed to console
 * 9 -- Return number of unread characters in the log buffer
 *     10 -- Return size of the log buffer
 */
int do_syslog(int type, char __user * buf, int len)
{
   unsigned long i, j, limit, count;
   int do_clear = 0;
   char c;
   int error = 0;

   error = security_syslog(type);
   if (error)
      return error;

   switch (type) {
   case 0:     /* Close log */
      break;
   case 1:     /* Open log */
      break;
   case 2:     /* Read from log */
      error = -EINVAL;
      if (!buf || len < 0)
         goto out;
      error = 0;
      if (!len)
         goto out;
      error = verify_area(VERIFY_WRITE,buf,len);
      if (error)
         goto out;
      error = wait_event_interruptible(log_wait, (log_start - log_end));
      if (error)
         goto out;
      i = 0;
      spin_lock_irq(&logbuf_lock);
      while (!error && (log_start != log_end) && i < len) {
         c = LOG_BUF(log_start);
         log_start++;
         spin_unlock_irq(&logbuf_lock);
         error = __put_user(c,buf);
         buf++;
         i++;
         spin_lock_irq(&logbuf_lock);
      }
      spin_unlock_irq(&logbuf_lock);
      if (!error)
         error = i;
      break;
   case 4:     /* Read/clear last kernel messages */
      do_clear = 1;
      /* FALL THRU */
   case 3:     /* Read last kernel messages */
      error = -EINVAL;
      if (!buf || len < 0)
         goto out;
      error = 0;
      if (!len)
         goto out;
      error = verify_area(VERIFY_WRITE,buf,len);
      if (error)
         goto out;
      count = len;
      if (count > log_buf_len)
         count = log_buf_len;
      spin_lock_irq(&logbuf_lock);
      if (count > logged_chars)
         count = logged_chars;
      if (do_clear)
         logged_chars = 0;
      limit = log_end;
      /*
       * __put_user() could sleep, and while we sleep
       * printk() could overwrite the messages
       * we try to copy to user space. Therefore
       * the messages are copied in reverse. <manfreds>
       */
      for(i = 0; i < count && !error; i++) {
         j = limit-1-i;
         if (j + log_buf_len < log_end)
            break;
         c = LOG_BUF(j);
         spin_unlock_irq(&logbuf_lock);
         error = __put_user(c,&buf[count-1-i]);
         spin_lock_irq(&logbuf_lock);
      }
      spin_unlock_irq(&logbuf_lock);
      if (error)
         break;
      error = i;
      if(i != count) {
         int offset = count-error;
         /* buffer overflow during copy, correct user buffer. */
         for(i=0;i<error;i++) {
            if (__get_user(c,&buf[i+offset]) ||
                __put_user(c,&buf[i])) {
               error = -EFAULT;
               break;
            }
         }
      }
      break;
   case 5:     /* Clear ring buffer */
      logged_chars = 0;
      break;
   case 6:     /* Disable logging to console */
      console_loglevel = minimum_console_loglevel;
      break;
   case 7:     /* Enable logging to console */
      console_loglevel = default_console_loglevel;
      break;
   case 8:     /* Set level of messages printed to console */
      error = -EINVAL;
      if (len < 1 || len > 8)
         goto out;
      if (len < minimum_console_loglevel)
         len = minimum_console_loglevel;
      console_loglevel = len;
      error = 0;
      break;
   case 9:     /* Number of chars in the log buffer */
      error = log_end - log_start;
      break;
   case 10: /* Size of the log buffer */
      error = log_buf_len;
      break;
   default:
      error = -EINVAL;
      break;
   }
out:
   return error;
}
