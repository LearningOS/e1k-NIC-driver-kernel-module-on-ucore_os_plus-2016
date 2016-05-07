#ifndef __LINUX_ASSOC_ARRAY_H__
#define __LINUX_ASSOC_ARRAY_H__

#include <linux/types.h>

/*
 * Generic associative array.
 */
struct assoc_array {
	struct assoc_array_ptr	*root;		/* The node at the root of the tree */
	unsigned long		nr_leaves_on_tree;
};

#endif /* ! __LINUX_ASSOC_ARRAY_H__ */
