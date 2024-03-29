#ifndef _LINUX_INIT_H
#define _LINUX_INIT_H

#include <linux/config.h>
#include <linux/compiler.h>
#ifdef TARGET_OS2
#include <linux/list.h>
#include <linux/stringify.h>
#endif

/* These macros are used to mark some functions or
 * initialized data (doesn't apply to uninitialized data)
 * as `initialization' functions. The kernel can take this
 * as hint that the function is used only during the initialization
 * phase and free up used memory resources after
 *
 * Usage:
 * For functions:
 *
 * You should add __init immediately before the function name, like:
 *
 * static void __init initme(int x, int y)
 * {
 *    extern int z; z = x * y;
 * }
 *
 * If the function has a prototype somewhere, you can also add
 * __init between closing brace of the prototype and semicolon:
 *
 * extern int initialize_foobar_device(int, int, int) __init;
 *
 * For initialized data:
 * You should insert __initdata between the variable name and equal
 * sign followed by value, e.g.:
 *
 * static int init_variable __initdata = 0;
 * static char linux_logo[] __initdata = { 0x32, 0x36, ... };
 *
 * Don't forget to initialize data not at file scope, i.e. within a function,
 * as gcc otherwise puts the data into the bss section and not into the init
 * section.
 *
 * Also note, that this data cannot be "const".
 */

/* These are for everybody (although not all archs will actually
   discard it in modules) */
#ifdef TARGET_OS2_AOUT
#define __init
#define __initdata
#define __exitdata
#define __exit_call   __attribute_used__
#ifdef LXKERNEL
#define __define_initcall(level,fn) \
 int LXRX_INITCALL_##level##fn(void) \
 { return fn(); } \
 EXPORT_SYMBOL(LXRX_INITCALL_##level##fn);

#define __exitcall(fn) \
void LXRX_EXITCALL_##fn(void) \
 { fn(); } \
 EXPORT_SYMBOL(LXRX_EXITCALL_##fn);

#define console_initcall(fn) \
 int LXRX_CONS_INITCALL_##fn(void) \
 { return fn(); } \
 EXPORT_SYMBOL(LXRX_CONS_INITCALL_##fn);

#endif // LXKERNEL
#else // TARGET_OS2_AOUT

#define __init    __attribute__ ((__section__ (".init.text")))
#define __initdata   __attribute__ ((__section__ (".init.data")))
#define __exitdata   __attribute__ ((__section__(".exit.data")))
#define __exit_call  __attribute_used__ __attribute__ ((__section__ (".exitcall.exit")))
#endif // TARGET_OS2_AOUT

#ifdef TARGET_OS2_AOUT
#define __exit
#define __sched
#else // TARGET_OS2_AOUT
#define __sched      __attribute__((__section__(".sched.text")))
#ifdef MODULE
#define __exit    __attribute__ ((__section__(".exit.text")))
#else
#define __exit    __attribute_used__ __attribute__ ((__section__(".exit.text")))
#endif
#endif

/* For assembly routines */
#define __INIT    .section ".init.text","ax"
#define __FINIT      .previous
#define __INITDATA   .section ".init.data","aw"

#ifndef __ASSEMBLY__
/*
 * Used for initialization calls..
 */
typedef int (*initcall_t)(void);
typedef void (*exitcall_t)(void);

extern initcall_t __con_initcall_start, __con_initcall_end;
extern initcall_t __security_initcall_start, __security_initcall_end;
#endif

#ifndef MODULE

#ifndef __ASSEMBLY__

/* initcalls are now grouped by functionality into separate
 * subsections. Ordering inside the subsections is determined
 * by link order.
 * For backwards compatibility, initcall() puts the call in
 * the device init subsection.
 */

#ifdef TARGET_OS2_AOUT
#define core_initcall(fn)     __define_initcall(1,fn)
#define postcore_initcall(fn)    __define_initcall(2,fn)
#define arch_initcall(fn)     __define_initcall(3,fn)
#define subsys_initcall(fn)      __define_initcall(4,fn)
#define fs_initcall(fn)       __define_initcall(5,fn)
#define device_initcall(fn)      __define_initcall(6,fn)
#define late_initcall(fn)     __define_initcall(7,fn)
#ifdef MODULE
#define module_init(fn) core_initcall(fn)
#endif
#else // !TARGET_OS2_AOUT
#define __define_initcall(level,fn) \
   static initcall_t __initcall_##fn __attribute_used__ \
   __attribute__((__section__(".initcall" level ".init"))) = fn

#define core_initcall(fn)     __define_initcall("1",fn)
#define postcore_initcall(fn)    __define_initcall("2",fn)
#define arch_initcall(fn)     __define_initcall("3",fn)
#define subsys_initcall(fn)      __define_initcall("4",fn)
#define fs_initcall(fn)       __define_initcall("5",fn)
#define device_initcall(fn)      __define_initcall("6",fn)
#define late_initcall(fn)     __define_initcall("7",fn)
#endif // TARGET_OS2_AOUT

#define __initcall(fn) device_initcall(fn)

#ifndef TARGET_OS2_AOUT
#define __exitcall(fn) \
   static exitcall_t __exitcall_##fn __exit_call = fn
#endif

#ifndef TARGET_OS2_AOUT
#define console_initcall(fn) \
   static initcall_t __initcall_##fn \
   __attribute_used__ __attribute__((__section__(".con_initcall.init")))=fn
#endif

#define security_initcall(fn) \
   static initcall_t __initcall_##fn \
   __attribute_used__ __attribute__((__section__(".security_initcall.init"))) = fn

struct obs_kernel_param {
   const char *str;
   int (*setup_func)(char *);
};

/* OBSOLETE: see moduleparam.h for the right way. */
#if defined(TARGET_OS2) && defined(TARGET_OS2_AOUT)
#ifdef TARGET_OS2_GNU2
// Because a const char is placed in the text section we can
// do a jmp to the real symbol without specifying .text
// (SM)
#define __setup_param(str, unique_id, fn) \
   const char __setup_str_##unique_id[] __initdata = str; \
   asm ( \
   ".globl " __stringify(_LXRX_SETUP_##unique_id)"\n\t" \
   __stringify(_LXRX_SETUP_##unique_id) ":\n\t" \
   "jmp " __stringify(fn) "\n\t" \
   );
#else
// Because a const char is placed in the text section we can
// do a jmp to the real symbol without specifying .text
// (SM)
#define __setup_param(str, unique_id, fn) \
   const char __setup_str_##unique_id[] __initdata = str; \
   asm ( \
   ".globl " __stringify(_LXRX_SETUP_##unique_id)"\n\t" \
   __stringify(_LXRX_SETUP_##unique_id) ":\n\t" \
   "jmp _" __stringify(fn) "\n\t" \
   );
#endif
#else
#define __setup_param(str, unique_id, fn)       \
   static char __setup_str_##unique_id[] __initdata = str;  \
   static struct obs_kernel_param __setup_##unique_id \
       __attribute_used__           \
       __attribute__((__section__(".init.setup"))) \
      = { __setup_str_##unique_id, fn }
#endif

#define __setup_null_param(str, unique_id)         \
   __setup_param(str, unique_id, NULL)

#define __setup(str, fn)               \
   __setup_param(str, fn, fn)

#define __obsolete_setup(str)             \
   __setup_null_param(str, __LINE__)

#endif /* __ASSEMBLY__ */

/**
 * module_init() - driver initialization entry point
 * @x: function to be run at kernel boot time or module insertion
 *
 * module_init() will either be called during do_initcalls (if
 * builtin) or at module insertion time (if a module).  There can only
 * be one per module.
 */
#define module_init(x)  __initcall(x);

/**
 * module_exit() - driver exit entry point
 * @x: function to be run when driver is removed
 *
 * module_exit() will wrap the driver clean-up code
 * with cleanup_module() when used with rmmod when
 * the driver is a module.  If the driver is statically
 * compiled into the kernel, module_exit() has no effect.
 * There can only be one per module.
 */
#define module_exit(x)  __exitcall(x);

#else /* MODULE */

/* Don't use these in modules, but some people do... */
#if (defined(TARGET_OS2) && defined(LXKERNEL))
#define core_initcall(fn)     __define_initcall(1,fn)
#define postcore_initcall(fn)    __define_initcall(2,fn)
#define arch_initcall(fn)     __define_initcall(3,fn)
#define subsys_initcall(fn)      __define_initcall(4,fn)
#define fs_initcall(fn)       __define_initcall(5,fn)
#define device_initcall(fn)      __define_initcall(6,fn)
#define late_initcall(fn)     __define_initcall(7,fn)
#define __initcall(fn) device_initcall(fn)
#else
#define core_initcall(fn)     module_init(fn)
#define postcore_initcall(fn)    module_init(fn)
#define arch_initcall(fn)     module_init(fn)
#define subsys_initcall(fn)      module_init(fn)
#define fs_initcall(fn)       module_init(fn)
#define device_initcall(fn)      module_init(fn)
#define late_initcall(fn)     module_init(fn)

#define security_initcall(fn)    module_init(fn)
#endif

/* These macros create a dummy inline: gcc 2.9x does not count alias
 as usage, hence the `unused function' warning when __init functions
 are declared static. We use the dummy __*_module_inline functions
 both to kill the warning and check the type of the init/cleanup
 function. */

/* Each module must use one module_init(), or one no_module_init */
#if (defined(TARGET_OS2) && defined(LXKERNEL))
#define module_init(fn) __initcall(fn)
#define module_exit(fn) __exitcall(fn)
#else
#define module_init(initfn)               \
   static inline initcall_t __inittest(void)    \
   { return initfn; }               \
   int init_module(void) __attribute__((alias(#initfn)));

/* This is only required if you want to be unloadable. */
#define module_exit(exitfn)               \
   static inline exitcall_t __exittest(void)    \
   { return exitfn; }               \
   void cleanup_module(void) __attribute__((alias(#exitfn)));
#endif

#if (defined(TARGET_OS2) && defined(LXKERNEL))
struct obs_kernel_param {
   const char *str;
   int (*setup_func)(char *);
};
#define __setup_param(str, unique_id, fn)       \
   static char __setup_str_##unique_id[] __initdata = str; \
   struct obs_kernel_param LXRX_SETUP_##unique_id \
      = { __setup_str_##unique_id, fn }
#define __setup_null_param(str, unique_id)         \
   __setup_param(str, unique_id, NULL)

#define __setup(str, fn)               \
   __setup_param(str, fn, fn)

#define __obsolete_setup(str)             \
   __setup_null_param(str, __LINE__)
#endif
#endif

/* Data marked not to be saved by software_suspend() */
#ifdef TARGET_OS2
#define __nosavedata
#else
#define __nosavedata __attribute__ ((__section__ (".data.nosave")))
#endif

/* This means "can be init if no module support, otherwise module load
   may call it." */
#ifdef CONFIG_MODULES
#define __init_or_module
#define __initdata_or_module
#else
#define __init_or_module __init
#define __initdata_or_module __initdata
#endif /*CONFIG_MODULES*/

#ifdef CONFIG_HOTPLUG
#define __devinit
#define __devinitdata
#define __devexit
#define __devexitdata
#else
#define __devinit __init
#define __devinitdata __initdata
#define __devexit __exit
#define __devexitdata __exitdata
#endif

/* Functions marked as __devexit may be discarded at kernel link time, depending
   on config options.  Newer versions of binutils detect references from
   retained sections to discarded sections and flag an error.  Pointers to
   __devexit functions must use __devexit_p(function_name), the wrapper will
   insert either the function_name or NULL, depending on the config options.
 */
#if defined(MODULE) || defined(CONFIG_HOTPLUG)
#define __devexit_p(x) x
#else
#define __devexit_p(x) NULL
#endif

#ifdef MODULE
#define __exit_p(x) x
#else
#define __exit_p(x) NULL
#endif

#endif /* _LINUX_INIT_H */
