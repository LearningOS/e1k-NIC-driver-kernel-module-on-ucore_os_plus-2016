#ifndef __LINUX_LOCKDEP_H__
#define __LINUX_LOCKDEP_H__

struct task_struct;

# define lock_acquire(l, s, t, r, c, n, i)	do { } while (0)
# define lock_release(l, n, i)			do { } while (0)

# define lockdep_init_map(lock, name, key, sub) \
		do { (void)(name); (void)(key); } while (0)

/*
 * The class key takes no space if lockdep is disabled:
 */
struct lock_class_key { };

#define LOCK_CONTENDED(_lock, try, lock) \
	lock(_lock)

#define LOCK_CONTENDED_FLAGS(_lock, try, lock, lockfl, flags) \
	lockfl((_lock), (flags))

#define lock_acquire_exclusive(l, s, t, n, i)		lock_acquire(l, s, t, 0, 1, n, i)

#define lock_acquire_shared_recursive(l, s, t, n, i)	lock_acquire(l, s, t, 2, 1, n, i)

#define spin_acquire(l, s, t, i)		lock_acquire_exclusive(l, s, t, NULL, i)

#define spin_release(l, n, i)			lock_release(l, n, i)

#define rwlock_acquire(l, s, t, i)		lock_acquire_exclusive(l, s, t, NULL, i)
#define rwlock_acquire_read(l, s, t, i)		lock_acquire_shared_recursive(l, s, t, NULL, i)
#define rwlock_release(l, n, i)			lock_release(l, n, i)

#define seqcount_acquire(l, s, t, i)		lock_acquire_exclusive(l, s, t, NULL, i)

#define seqcount_release(l, n, i)		lock_release(l, n, i)

#endif /* ! __LINUX_LOCKDEP_H__ */
