#ifndef __LINUX_PREEMPT_H__
#define __LINUX_PREEMPT_H__

#include <linux/linkage.h>
#include <linux/list.h>

/*
 * We use the MSB mostly because its available; see <linux/preempt_mask.h> for
 * the other bits -- can't include that header due to inclusion hell.
 */
#define PREEMPT_NEED_RESCHED	0x80000000

#include <asm/preempt.h>

#define preempt_count_add(val)	__preempt_count_add(val)

/*
 * Even if we don't have any preemption, we need preempt disable/enable
 * to be barriers, so that we don't have things like get_user/put_user
 * that can cause faults and scheduling migrate into our preempt-protected
 * region.
 */
#define preempt_disable()			barrier()

#define preempt_enable()			barrier()

#define preempt_disable_notrace()		barrier()

#define preempt_enable_notrace()		barrier()

#define preempt_fold_need_resched() \
do { \
	if (tif_need_resched()) \
		set_preempt_need_resched(); \
} while (0)

#endif /* ! __LINUX_PREEMPT_H__ */
