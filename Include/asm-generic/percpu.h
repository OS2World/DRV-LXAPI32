#ifndef _ASM_GENERIC_PERCPU_H_
#define _ASM_GENERIC_PERCPU_H_
#include <linux/compiler.h>

#ifdef TARGET_OS2
#ifndef NR_CPUS
#include <linux/threads.h>
#endif
#endif

#define __GENERIC_PER_CPU
#ifdef CONFIG_SMP

#ifndef TARGET_OS2
extern unsigned long __per_cpu_offset[NR_CPUS];
#endif /* ! TARGET_OS2 */

/* Separate out the type, so (int[3], foo) works. */
#ifdef TARGET_OS2
#define DEFINE_PER_CPU(type, name) \
    unsigned long LXRX_SZ_PERCPU_##name=sizeof(__typeof__(type)); \
    __typeof__(type) per_cpu__##name[NR_CPUS]; \
    __typeof__(type) LXRX_PERCPU__##name
#else
#define DEFINE_PER_CPU(type, name) \
    __attribute__((__section__(".data.percpu"))) __typeof__(type) per_cpu__##name
#endif /* ! TARGET_OS2 */

/* var is in discarded region: offset to particular copy we want */
#ifdef TARGET_OS2
#define per_cpu(var, cpu) (per_cpu__##var[cpu])
#else
#define per_cpu(var, cpu) (*RELOC_HIDE(&per_cpu__##var, __per_cpu_offset[cpu]))
#endif /* ! TARGET_OS2 */
#define __get_cpu_var(var) per_cpu(var, smp_processor_id())

/* A macro to avoid #include hell... */
#ifdef TARGET_OS2
#define percpu_modcopy(pcpudst, src, size) // TODO (SM)
#else
#define percpu_modcopy(pcpudst, src, size)         \
do {                       \
   unsigned int __i;             \
   for (__i = 0; __i < NR_CPUS; __i++)       \
      if (cpu_possible(__i))           \
         memcpy((pcpudst)+__per_cpu_offset[__i],   \
                (src), (size));        \
} while (0)
#endif /* ! TARGET_OS2 */
#else /* ! SMP */

#define DEFINE_PER_CPU(type, name) \
    __typeof__(type) per_cpu__##name

#define per_cpu(var, cpu)        (*((void)cpu, &per_cpu__##var))
#define __get_cpu_var(var)       per_cpu__##var

#endif   /* SMP */

#ifdef TARGET_OS2
#define DECLARE_PER_CPU(type, name) \
 extern __typeof__(type) per_cpu__##name[]; \
 extern __typeof__(type) LXRX_PERCPU__##name;
#else
#define DECLARE_PER_CPU(type, name) extern __typeof__(type) per_cpu__##name
#endif /* ! TARGET_OS2 */

#define EXPORT_PER_CPU_SYMBOL(var) EXPORT_SYMBOL(per_cpu__##var)
#define EXPORT_PER_CPU_SYMBOL_GPL(var) EXPORT_SYMBOL_GPL(per_cpu__##var)

#endif /* _ASM_GENERIC_PERCPU_H_ */
