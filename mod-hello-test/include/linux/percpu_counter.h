#ifndef __LINUX_PERCPU_COUNTER_H__
#define __LINUX_PERCPU_COUNTER_H__

#include <linux/spinlock.h>
#include <linux/smp.h>
#include <linux/list.h>
#include <linux/threads.h>
#include <linux/percpu.h>
#include <linux/types.h>

struct percpu_counter {
	raw_spinlock_t lock;
	s64 count;
#ifdef CONFIG_HOTPLUG_CPU
	struct list_head list;	/* All percpu_counters are on a list */
#endif
	s32 __percpu *counters;
};

extern int percpu_counter_batch;

void __percpu_counter_add(struct percpu_counter *fbc, s64 amount, s32 batch);
s64 __percpu_counter_sum(struct percpu_counter *fbc);

static inline void percpu_counter_add(struct percpu_counter *fbc, s64 amount)
{
	__percpu_counter_add(fbc, amount, percpu_counter_batch);
}

static inline void percpu_counter_inc(struct percpu_counter *fbc)
{
	percpu_counter_add(fbc, 1);
}

static inline void percpu_counter_dec(struct percpu_counter *fbc)
{
	percpu_counter_add(fbc, -1);
}

#endif /* ! __LINUX_PERCPU_COUNTER_H__ */
