#ifndef __LINUX_SEQLOCK_H__
#define __LINUX_SEQLOCK_H__

#include <linux/spinlock.h>
#include <linux/preempt.h>
#include <linux/lockdep.h>
#include <asm/processor.h>

/*
 * Version using sequence counter only.
 * This can be used when code has its own mutex protecting the
 * updating starting before the write_seqcountbeqin() and ending
 * after the write_seqcount_end().
 */
typedef struct seqcount {
	unsigned sequence;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map dep_map;
#endif
} seqcount_t;

# define seqcount_lockdep_reader_access(x)

/**
 * __read_seqcount_begin - begin a seq-read critical section (without barrier)
 * @s: pointer to seqcount_t
 * Returns: count to be passed to read_seqcount_retry
 *
 * __read_seqcount_begin is like read_seqcount_begin, but has no smp_rmb()
 * barrier. Callers should ensure that smp_rmb() or equivalent ordering is
 * provided before actually loading any of the variables that are to be
 * protected in this critical section.
 *
 * Use carefully, only in critical code, and comment how the barrier is
 * provided.
 */
static inline unsigned __read_seqcount_begin(const seqcount_t *s)
{
	unsigned ret;

repeat:
	ret = ACCESS_ONCE(s->sequence);
	if (unlikely(ret & 1)) {
		cpu_relax();
		goto repeat;
	}
	return ret;
}

/**
 * raw_read_seqcount_begin - start seq-read critical section w/o lockdep
 * @s: pointer to seqcount_t
 * Returns: count to be passed to read_seqcount_retry
 *
 * raw_read_seqcount_begin opens a read critical section of the given
 * seqcount, but without any lockdep checking. Validity of the critical
 * section is tested by checking read_seqcount_retry function.
 */
static inline unsigned raw_read_seqcount_begin(const seqcount_t *s)
{
	unsigned ret = __read_seqcount_begin(s);
	smp_rmb();
	return ret;
}

/**
 * read_seqcount_begin - begin a seq-read critical section
 * @s: pointer to seqcount_t
 * Returns: count to be passed to read_seqcount_retry
 *
 * read_seqcount_begin opens a read critical section of the given seqcount.
 * Validity of the critical section is tested by checking read_seqcount_retry
 * function.
 */
static inline unsigned read_seqcount_begin(const seqcount_t *s)
{
	seqcount_lockdep_reader_access(s);
	return raw_read_seqcount_begin(s);
}

/**
 * __read_seqcount_retry - end a seq-read critical section (without barrier)
 * @s: pointer to seqcount_t
 * @start: count, from read_seqcount_begin
 * Returns: 1 if retry is required, else 0
 *
 * __read_seqcount_retry is like read_seqcount_retry, but has no smp_rmb()
 * barrier. Callers should ensure that smp_rmb() or equivalent ordering is
 * provided before actually loading any of the variables that are to be
 * protected in this critical section.
 *
 * Use carefully, only in critical code, and comment how the barrier is
 * provided.
 */
static inline int __read_seqcount_retry(const seqcount_t *s, unsigned start)
{
	return unlikely(s->sequence != start);
}

/**
 * read_seqcount_retry - end a seq-read critical section
 * @s: pointer to seqcount_t
 * @start: count, from read_seqcount_begin
 * Returns: 1 if retry is required, else 0
 *
 * read_seqcount_retry closes a read critical section of the given seqcount.
 * If the critical section was invalid, it must be ignored (and typically
 * retried).
 */
static inline int read_seqcount_retry(const seqcount_t *s, unsigned start)
{
	smp_rmb();
	return __read_seqcount_retry(s, start);
}

static inline void raw_write_seqcount_begin(seqcount_t *s)
{
	s->sequence++;
	smp_wmb();
}

static inline void raw_write_seqcount_end(seqcount_t *s)
{
	smp_wmb();
	s->sequence++;
}

/*
 * Sequence counter only version assumes that callers are using their
 * own mutexing.
 */
static inline void write_seqcount_begin_nested(seqcount_t *s, int subclass)
{
	raw_write_seqcount_begin(s);
	seqcount_acquire(&s->dep_map, subclass, 0, _RET_IP_);
}

static inline void write_seqcount_begin(seqcount_t *s)
{
	write_seqcount_begin_nested(s, 0);
}

static inline void write_seqcount_end(seqcount_t *s)
{
	seqcount_release(&s->dep_map, 1, _RET_IP_);
	raw_write_seqcount_end(s);
}

typedef struct {
	struct seqcount seqcount;
	spinlock_t lock;
} seqlock_t;

/*
 * Read side functions for starting and finalizing a read side section.
 */
static inline unsigned read_seqbegin(const seqlock_t *sl)
{
	return read_seqcount_begin(&sl->seqcount);
}

static inline unsigned read_seqretry(const seqlock_t *sl, unsigned start)
{
	return read_seqcount_retry(&sl->seqcount, start);
}

/*
 * A locking reader exclusively locks out other writers and locking readers,
 * but doesn't update the sequence number. Acts like a normal spin_lock/unlock.
 * Don't need preempt_disable() because that is in the spin_lock already.
 */
static inline void read_seqlock_excl(seqlock_t *sl)
{
	spin_lock(&sl->lock);
}

static inline void read_sequnlock_excl(seqlock_t *sl)
{
	spin_unlock(&sl->lock);
}

#endif /* ! __LINUX_SEQLOCK_H__ */
