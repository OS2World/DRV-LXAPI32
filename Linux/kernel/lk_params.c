/* $Id: lk_params.c,v 1.1 2004/07/19 22:36:43 smilcke Exp $ */

/*
 * params.c
 * Autor:               Stefan Milcke
 * Erstellt am:         02.05.2004
 * Letzte Aenderung am: 28.05.2004
 *
*/

#include <lxcommon.h>

#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/module.h>

#if 0
#define DEBUGP printk
#else
#define DEBUGP(fmt, a...)
#endif

//------------------------------ dash2underscore -------------------------------
static inline int dash2underscore(char c)
{
   if (c == '-')
      return '_';
   return c;
}

//---------------------------------- parameq -----------------------------------
static inline int parameq(const char *input, const char *paramname)
{
   unsigned int i;
   for (i = 0; dash2underscore(input[i]) == paramname[i]; i++)
      if (input[i] == '\0')
         return 1;
   return 0;
}

//--------------------------------- parse_one ----------------------------------
static int parse_one(char *param,
           char *val,
           struct kernel_param *params,
           unsigned num_params,
           int (*handle_unknown)(char *param, char *val))
{
   unsigned int i;

   /* Find parameter */
   for (i = 0; i < num_params; i++) {
      if (parameq(param, params[i].name)) {
         DEBUGP("They are equal!  Calling %p\n",
                params[i].set);
         return params[i].set(val, &params[i]);
      }
   }

   if (handle_unknown) {
      DEBUGP("Unknown argument: calling %p\n", handle_unknown);
      return handle_unknown(param, val);
   }

   DEBUGP("Unknown argument `%s'\n", param);
   return -ENOENT;
}

/* You can use " around spaces, but can't escape ". */
/* Hyphens and underscores equivalent in parameter names. */
//--------------------------------- *next_arg ----------------------------------
static char *next_arg(char *args, char **param, char **val)
{
   unsigned int i, equals = 0;
   int in_quote = 0;

   /* Chew any extra spaces */
   while (*args == ' ') args++;

   for (i = 0; args[i]; i++) {
      if (args[i] == ' ' && !in_quote)
         break;
      if (equals == 0) {
         if (args[i] == '=')
            equals = i;
      }
      if (args[i] == '"')
         in_quote = !in_quote;
   }

   *param = args;
   if (!equals)
      *val = NULL;
   else {
      args[equals] = '\0';
      *val = args + equals + 1;
   }

   if (args[i]) {
      args[i] = '\0';
      return args + i + 1;
   } else
      return args + i;
}

/* Args looks like "foo=bar,bar2 baz=fuz wiz". */
//--------------------------------- parse_args ---------------------------------
int parse_args(const char *name,
          char *args,
          struct kernel_param *params,
          unsigned num,
          int (*unknown)(char *param, char *val))
{
   char *param, *val;

   DEBUGP("Parsing ARGS: %s\n", args);

   while (*args) {
      int ret;

      args = next_arg(args, &param, &val);
      ret = parse_one(param, val, params, num, unknown);
      switch (ret) {
      case -ENOENT:
         printk(KERN_ERR "%s: Unknown parameter `%s'\n",
                name, param);
         return ret;
      case -ENOSPC:
         printk(KERN_ERR
                "%s: `%s' too large for parameter `%s'\n",
                name, val ?: "", param);
         return ret;
      case 0:
         break;
      default:
         printk(KERN_ERR
                "%s: `%s' invalid for parameter `%s'\n",
                name, val ?: "", param);
         return ret;
      }
   }

   /* All parsed OK. */
   return 0;
}

/* Lazy bastard, eh? */
#define STANDARD_PARAM_DEF(name, type, format, tmptype, strtolfn)       \
   int param_set_##name(const char *val, struct kernel_param *kp) \
   {                       \
      char *endp;                \
      tmptype l;                 \
                           \
      if (!val) return -EINVAL;           \
      l = strtolfn(val, &endp, 0);           \
      if (endp == val || *endp || ((type)l != l))     \
         return -EINVAL;               \
      *((type *)kp->arg) = l;             \
      return 0;                  \
   }                       \
   int param_get_##name(char *buffer, struct kernel_param *kp) \
   {                       \
      return sprintf(buffer, format, *((type *)kp->arg));   \
   }

STANDARD_PARAM_DEF(short, short, "%hi", long, simple_strtol);
STANDARD_PARAM_DEF(ushort, unsigned short, "%hu", unsigned long, simple_strtoul);
STANDARD_PARAM_DEF(int, int, "%i", long, simple_strtol);
STANDARD_PARAM_DEF(uint, unsigned int, "%u", unsigned long, simple_strtoul);
STANDARD_PARAM_DEF(long, long, "%li", long, simple_strtol);
STANDARD_PARAM_DEF(ulong, unsigned long, "%lu", unsigned long, simple_strtoul);

//------------------------------ param_set_charp -------------------------------
int param_set_charp(const char *val, struct kernel_param *kp)
{
   if (!val) {
      printk(KERN_ERR "%s: string parameter expected\n",
             kp->name);
      return -EINVAL;
   }

   if (strlen(val) > 1024) {
      printk(KERN_ERR "%s: string parameter too long\n",
             kp->name);
      return -ENOSPC;
   }

   *(char **)kp->arg = (char *)val;
   return 0;
}

//------------------------------ param_get_charp -------------------------------
int param_get_charp(char *buffer, struct kernel_param *kp)
{
   return sprintf(buffer, "%s", *((char **)kp->arg));
}

int param_set_bool(const char *val, struct kernel_param *kp)
{
   /* No equals means "set"... */
   if (!val) val = "1";

   /* One of =[yYnN01] */
   switch (val[0]) {
   case 'y': case 'Y': case '1':
      *(int *)kp->arg = 1;
      return 0;
   case 'n': case 'N': case '0':
      *(int *)kp->arg = 0;
      return 0;
   }
   return -EINVAL;
}

//------------------------------- param_get_bool -------------------------------
int param_get_bool(char *buffer, struct kernel_param *kp)
{
   /* Y and N chosen as being relatively non-coder friendly */
   return sprintf(buffer, "%c", (*(int *)kp->arg) ? 'Y' : 'N');
}

//----------------------------- param_set_invbool ------------------------------
int param_set_invbool(const char *val, struct kernel_param *kp)
{
   int boolval, ret;
   struct kernel_param dummy = { .arg = &boolval };

   ret = param_set_bool(val, &dummy);
   if (ret == 0)
      *(int *)kp->arg = !boolval;
   return ret;
}

//----------------------------- param_get_invbool ------------------------------
int param_get_invbool(char *buffer, struct kernel_param *kp)
{
   int val;
   struct kernel_param dummy = { .arg = &val };

   val = !*(int *)kp->arg;
   return param_get_bool(buffer, &dummy);
}

/* We cheat here and temporarily mangle the string. */
//-------------------------------- param_array ---------------------------------
int param_array(const char *name,
      const char *val,
      unsigned int min, unsigned int max,
      void *elem, int elemsize,
      int (*set)(const char *, struct kernel_param *kp),
      int *num)
{
   int ret;
   struct kernel_param kp;
   char save;

   /* Get the name right for errors. */
   kp.name = name;
   kp.arg = elem;

   /* No equals sign? */
   if (!val) {
      printk(KERN_ERR "%s: expects arguments\n", name);
      return -EINVAL;
   }

   *num = 0;
   /* We expect a comma-separated list of values. */
   do {
      int len;

      if (*num == max) {
         printk(KERN_ERR "%s: can only take %i arguments\n",
                name, max);
         return -EINVAL;
      }
      len = strcspn(val, ",");

      /* nul-terminate and parse */
      save = val[len];
      ((char *)val)[len] = '\0';
      ret = set(val, &kp);

      if (ret != 0)
         return ret;
      kp.arg += elemsize;
      val += len+1;
      (*num)++;
   } while (save == ',');

   if (*num < min) {
      printk(KERN_ERR "%s: needs at least %i arguments\n",
             name, min);
      return -EINVAL;
   }
   return 0;
}

//------------------------------ param_array_set -------------------------------
int param_array_set(const char *val, struct kernel_param *kp)
{
   struct kparam_array *arr = kp->arg;

   return param_array(kp->name, val, 1, arr->max, arr->elem,
            arr->elemsize, arr->set, arr->num);
}

//------------------------------ param_array_get -------------------------------
int param_array_get(char *buffer, struct kernel_param *kp)
{
   int i, off, ret;
   struct kparam_array *arr = kp->arg;
   struct kernel_param p;

   p = *kp;
   for (i = off = 0; i < *arr->num; i++) {
      if (i)
         buffer[off++] = ',';
      p.arg = arr->elem + arr->elemsize * i;
      ret = arr->get(buffer + off, &p);
      if (ret < 0)
         return ret;
      off += ret;
   }
   buffer[off] = '\0';
   return off;
}

//---------------------------- param_set_copystring ----------------------------
int param_set_copystring(const char *val, struct kernel_param *kp)
{
   struct kparam_string *kps = kp->arg;

   if (strlen(val)+1 > kps->maxlen) {
      printk(KERN_ERR "%s: string doesn't fit in %u chars.\n",
             kp->name, kps->maxlen-1);
      return -ENOSPC;
   }
   strcpy(kps->string, val);
   return 0;
}

EXPORT_SYMBOL(param_set_short);
EXPORT_SYMBOL(param_get_short);
EXPORT_SYMBOL(param_set_ushort);
EXPORT_SYMBOL(param_get_ushort);
EXPORT_SYMBOL(param_set_int);
EXPORT_SYMBOL(param_get_int);
EXPORT_SYMBOL(param_set_uint);
EXPORT_SYMBOL(param_get_uint);
EXPORT_SYMBOL(param_set_long);
EXPORT_SYMBOL(param_get_long);
EXPORT_SYMBOL(param_set_ulong);
EXPORT_SYMBOL(param_get_ulong);
EXPORT_SYMBOL(param_set_charp);
EXPORT_SYMBOL(param_get_charp);
EXPORT_SYMBOL(param_set_bool);
EXPORT_SYMBOL(param_get_bool);
EXPORT_SYMBOL(param_set_invbool);
EXPORT_SYMBOL(param_get_invbool);
EXPORT_SYMBOL(param_array_set);
EXPORT_SYMBOL(param_array_get);
EXPORT_SYMBOL(param_set_copystring);
