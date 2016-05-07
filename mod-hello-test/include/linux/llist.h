#ifndef __LINUX_LLIST_H__
#define __LINUX_LLIST_H__

#include <linux/kernel.h>
#include <asm/cmpxchg.h>

struct llist_head {
	struct llist_node *first;
};

struct llist_node {
	struct llist_node *next;
};

extern bool llist_add_batch(struct llist_node *new_first,
			    struct llist_node *new_last,
			    struct llist_head *head);

#endif /* ! __LINUX_LLIST_H__ */
