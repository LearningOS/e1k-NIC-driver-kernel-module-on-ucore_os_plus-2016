#ifndef __LINUX_BOTTOM_HALF_H__
#define __LINUX_BOTTOM_HALF_H__

#include <linux/preempt.h>
#include <linux/preempt_mask.h>

static __always_inline void __local_bh_disable_ip(unsigned long ip, unsigned int cnt)
{
	preempt_count_add(cnt);
	barrier();
}

static inline void local_bh_disable(void)
{
	__local_bh_disable_ip(_THIS_IP_, SOFTIRQ_DISABLE_OFFSET);
}

extern void __local_bh_enable_ip(unsigned long ip, unsigned int cnt);

static inline void local_bh_enable(void)
{
	__local_bh_enable_ip(_THIS_IP_, SOFTIRQ_DISABLE_OFFSET);
}

#endif /* ! __LINUX_BOTTOM_HALF_H__ */
