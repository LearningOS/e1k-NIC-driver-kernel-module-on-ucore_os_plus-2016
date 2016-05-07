#ifndef __LINUX_LIST_LRU_H__
#define __LINUX_LIST_LRU_H__

#include <linux/list.h>
#include <linux/nodemask.h>

enum lru_status {
	LRU_REMOVED,		/* item removed from list */
	LRU_REMOVED_RETRY,	/* item removed, but lock has been
				   dropped and reacquired */
	LRU_ROTATE,		/* item referenced, give another pass */
	LRU_SKIP,		/* item cannot be locked, skip */
	LRU_RETRY,		/* item not freeable. May drop the lock
				   internally, but has to return locked. */
};

struct list_lru_node {
	spinlock_t		lock;
	struct list_head	list;
	/* kept as signed so we can catch imbalance bugs */
	long			nr_items;
} ____cacheline_aligned_in_smp;

struct list_lru {
	struct list_lru_node	*node;
	nodemask_t		active_nodes;
};

int list_lru_init_key(struct list_lru *lru, struct lock_class_key *key);

/**
 * list_lru_count_node: return the number of objects currently held by @lru
 * @lru: the lru pointer.
 * @nid: the node id to count from.
 *
 * Always return a non-negative number, 0 for empty lists. There is no
 * guarantee that the list is not updated while the count is being computed.
 * Callers that want such a guarantee need to provide an outer lock.
 */
unsigned long list_lru_count_node(struct list_lru *lru, int nid);

typedef enum lru_status
(*list_lru_walk_cb)(struct list_head *item, spinlock_t *lock, void *cb_arg);
/**
 * list_lru_walk_node: walk a list_lru, isolating and disposing freeable items.
 * @lru: the lru pointer.
 * @nid: the node id to scan from.
 * @isolate: callback function that is resposible for deciding what to do with
 *  the item currently being scanned
 * @cb_arg: opaque type that will be passed to @isolate
 * @nr_to_walk: how many items to scan.
 *
 * This function will scan all elements in a particular list_lru, calling the
 * @isolate callback for each of those items, along with the current list
 * spinlock and a caller-provided opaque. The @isolate callback can choose to
 * drop the lock internally, but *must* return with the lock held. The callback
 * will return an enum lru_status telling the list_lru infrastructure what to
 * do with the object being scanned.
 *
 * Please note that nr_to_walk does not mean how many objects will be freed,
 * just how many objects will be scanned.
 *
 * Return value: the number of objects effectively removed from the LRU.
 */
unsigned long list_lru_walk_node(struct list_lru *lru, int nid,
				 list_lru_walk_cb isolate, void *cb_arg,
				 unsigned long *nr_to_walk);

#endif /* ! __LINUX_LIST_LRU_H__ */
