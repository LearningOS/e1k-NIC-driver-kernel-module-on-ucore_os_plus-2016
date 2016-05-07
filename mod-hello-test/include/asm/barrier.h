#ifndef __ASM_BARRIER_H__
#define __ASM_BARRIER_H__

#include <asm/alternative.h>
#include <asm/nops.h>

#define mb() 	asm volatile("mfence":::"memory")

#define read_barrier_depends()	do { } while (0)

#define smp_mb()	mb()

# define smp_rmb()	barrier()

#define smp_wmb()	barrier()
#define smp_read_barrier_depends()	read_barrier_depends()

#define smp_store_release(p, v)						\
do {									\
	compiletime_assert_atomic_type(*p);				\
	barrier();							\
	ACCESS_ONCE(*p) = (v);						\
} while (0)

#define smp_mb__before_atomic()	barrier()
#define smp_mb__after_atomic()	barrier()

/*
 * Stop RDTSC speculation. This is needed when you need to use RDTSC
 * (or get_cycles or vread that possibly accesses the TSC) in a defined
 * code region.
 *
 * (Could use an alternative three way for this if there was one.)
 */
static __always_inline void rdtsc_barrier(void)
{
	alternative(ASM_NOP3, "mfence", X86_FEATURE_MFENCE_RDTSC);
	alternative(ASM_NOP3, "lfence", X86_FEATURE_LFENCE_RDTSC);
}

#endif /* ! __ASM_BARRIER_H__ */
