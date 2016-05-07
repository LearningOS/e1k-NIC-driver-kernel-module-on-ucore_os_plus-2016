#ifndef __LINUX_SCHED_PRIO_H__
#define __LINUX_SCHED_PRIO_H__

#define MAX_NICE	19
#define MIN_NICE	-20
#define NICE_WIDTH	(MAX_NICE - MIN_NICE + 1)

#define MAX_USER_RT_PRIO	100
#define MAX_RT_PRIO		MAX_USER_RT_PRIO

#define DEFAULT_PRIO		(MAX_RT_PRIO + NICE_WIDTH / 2)

#define PRIO_TO_NICE(prio)	((prio) - DEFAULT_PRIO)

#endif /* ! __LINUX_SCHED_PRIO_H__ */
