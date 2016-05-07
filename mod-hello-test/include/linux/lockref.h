#ifndef __LINUX_LOCKREF_H__
#define __LINUX_LOCKREF_H__

#include <linux/spinlock.h>
#include <generated/bounds.h>

#define USE_CMPXCHG_LOCKREF \
	(IS_ENABLED(CONFIG_ARCH_USE_CMPXCHG_LOCKREF) && \
	 IS_ENABLED(CONFIG_SMP) && SPINLOCK_SIZE <= 4)

struct lockref {
	union {
#if USE_CMPXCHG_LOCKREF
		aligned_u64 lock_count;
#endif
		struct {
			spinlock_t lock;
			unsigned int count;
		};
	};
};

extern void lockref_get(struct lockref *);

#endif /* ! __LINUX_LOCKREF_H__ */
