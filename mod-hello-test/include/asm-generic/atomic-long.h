#ifndef __ASM_GENERIC_ATOMIC_LONG_H__
#define __ASM_GENERIC_ATOMIC_LONG_H__

#include <asm/types.h>

typedef atomic64_t atomic_long_t;

static inline void atomic_long_inc(atomic_long_t *l)
{
	atomic64_t *v = (atomic64_t *)l;

	atomic64_inc(v);
}

#endif /* ! __ASM_GENERIC_ATOMIC_LONG_H__ */
