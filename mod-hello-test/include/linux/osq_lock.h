#ifndef __LINUX_OSQ_LOCK_H__
#define __LINUX_OSQ_LOCK_H__

#define OSQ_UNLOCKED_VAL (0)

struct optimistic_spin_queue {
	/*
	 * Stores an encoded value of the CPU # of the tail node in the queue.
	 * If the queue is empty, then it's set to OSQ_UNLOCKED_VAL.
	 */
	atomic_t tail;
};

#endif /* ! __LINUX_OSQ_LOCK_H__ */
