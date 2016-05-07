#ifndef __ASM_GENERIC_QRWLOCK_H__
#define __ASM_GENERIC_QRWLOCK_H__

#include <linux/atomic.h>
#include <asm/barrier.h>
#include <asm/processor.h>

#include <asm-generic/qrwlock_types.h>

#define	_QW_LOCKED	0xff		/* A writer holds the lock */
#define	_QW_WMASK	0xff		/* Writer mask		   */
#define	_QR_SHIFT	8		/* Reader count shift	   */
#define _QR_BIAS	(1U << _QR_SHIFT)

/*
 * External function declarations
 */
extern void queue_read_lock_slowpath(struct qrwlock *lock);
extern void queue_write_lock_slowpath(struct qrwlock *lock);

/**
 * queue_read_trylock - try to acquire read lock of a queue rwlock
 * @lock : Pointer to queue rwlock structure
 * Return: 1 if lock acquired, 0 if failed
 */
static inline int queue_read_trylock(struct qrwlock *lock)
{
	u32 cnts;

	cnts = atomic_read(&lock->cnts);
	if (likely(!(cnts & _QW_WMASK))) {
		cnts = (u32)atomic_add_return(_QR_BIAS, &lock->cnts);
		if (likely(!(cnts & _QW_WMASK)))
			return 1;
		atomic_sub(_QR_BIAS, &lock->cnts);
	}
	return 0;
}

/**
 * queue_write_trylock - try to acquire write lock of a queue rwlock
 * @lock : Pointer to queue rwlock structure
 * Return: 1 if lock acquired, 0 if failed
 */
static inline int queue_write_trylock(struct qrwlock *lock)
{
	u32 cnts;

	cnts = atomic_read(&lock->cnts);
	if (unlikely(cnts))
		return 0;

	return likely(atomic_cmpxchg(&lock->cnts,
				     cnts, cnts | _QW_LOCKED) == cnts);
}
/**
 * queue_read_lock - acquire read lock of a queue rwlock
 * @lock: Pointer to queue rwlock structure
 */
static inline void queue_read_lock(struct qrwlock *lock)
{
	u32 cnts;

	cnts = atomic_add_return(_QR_BIAS, &lock->cnts);
	if (likely(!(cnts & _QW_WMASK)))
		return;

	/* The slowpath will decrement the reader count, if necessary. */
	queue_read_lock_slowpath(lock);
}

/**
 * queue_write_lock - acquire write lock of a queue rwlock
 * @lock : Pointer to queue rwlock structure
 */
static inline void queue_write_lock(struct qrwlock *lock)
{
	/* Optimize for the unfair lock case where the fair flag is 0. */
	if (atomic_cmpxchg(&lock->cnts, 0, _QW_LOCKED) == 0)
		return;

	queue_write_lock_slowpath(lock);
}

/**
 * queue_read_unlock - release read lock of a queue rwlock
 * @lock : Pointer to queue rwlock structure
 */
static inline void queue_read_unlock(struct qrwlock *lock)
{
	/*
	 * Atomically decrement the reader count
	 */
	smp_mb__before_atomic();
	atomic_sub(_QR_BIAS, &lock->cnts);
}

#define arch_read_lock(l)	queue_read_lock(l)
#define arch_write_lock(l)	queue_write_lock(l)
#define arch_read_trylock(l)	queue_read_trylock(l)
#define arch_write_trylock(l)	queue_write_trylock(l)
#define arch_read_unlock(l)	queue_read_unlock(l)
#define arch_write_unlock(l)	queue_write_unlock(l)

#endif /* ! __ASM_GENERIC_QRWLOCK_H__ */
