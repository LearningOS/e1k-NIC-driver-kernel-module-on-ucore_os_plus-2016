#ifndef __LINUX_NODEMASK_H__
#define __LINUX_NODEMASK_H__

#include <linux/kernel.h>
#include <linux/threads.h>
#include <linux/bitmap.h>
#include <linux/numa.h>

typedef struct { DECLARE_BITMAP(bits, MAX_NUMNODES); } nodemask_t;

/*
 * The inline keyword gives the compiler room to decide to inline, or
 * not inline a function as it sees best.  However, as these functions
 * are called in both __init and non-__init functions, if they are not
 * inlined we will end up with a section mis-match error (of the type of
 * freeable items not being freed).  So we must use __always_inline here
 * to fix the problem.  If other functions in the future also end up in
 * this situation they will also need to be annotated as __always_inline
 */
#define node_set(node, dst) __node_set((node), &(dst))
static __always_inline void __node_set(int node, volatile nodemask_t *dstp)
{
	set_bit(node, dstp->bits);
}

static inline void __node_clear(int node, volatile nodemask_t *dstp)
{
	clear_bit(node, dstp->bits);
}

#define nodes_clear(dst) __nodes_clear(&(dst), MAX_NUMNODES)
static inline void __nodes_clear(nodemask_t *dstp, int nbits)
{
	bitmap_zero(dstp->bits, nbits);
}

#define node_isset(node, nodemask) test_bit((node), (nodemask).bits)

#define nodes_weight(nodemask) __nodes_weight(&(nodemask), MAX_NUMNODES)
static inline int __nodes_weight(const nodemask_t *srcp, int nbits)
{
	return bitmap_weight(srcp->bits, nbits);
}

#define first_node(src) __first_node(&(src))
static inline int __first_node(const nodemask_t *srcp)
{
	return min_t(int, MAX_NUMNODES, find_first_bit(srcp->bits, MAX_NUMNODES));
}

#define next_node(n, src) __next_node((n), &(src))
static inline int __next_node(int n, const nodemask_t *srcp)
{
	return min_t(int,MAX_NUMNODES,find_next_bit(srcp->bits, MAX_NUMNODES, n+1));
}

#define for_each_node_mask(node, mask)			\
	for ((node) = first_node(mask);			\
		(node) < MAX_NUMNODES;			\
		(node) = next_node((node), (mask)))

/*
 * Bitmasks that are kept for all the nodes.
 */
enum node_states {
	N_POSSIBLE,		/* The node could become online at some point */
	N_ONLINE,		/* The node is online */
	N_NORMAL_MEMORY,	/* The node has regular memory */
#ifdef CONFIG_HIGHMEM
	N_HIGH_MEMORY,		/* The node has regular or high memory */
#else
	N_HIGH_MEMORY = N_NORMAL_MEMORY,
#endif
#ifdef CONFIG_MOVABLE_NODE
	N_MEMORY,		/* The node has memory(regular, high, movable) */
#else
	N_MEMORY = N_HIGH_MEMORY,
#endif
	N_CPU,		/* The node has one or more cpus */
	NR_NODE_STATES
};

extern nodemask_t node_states[NR_NODE_STATES];

static inline int node_state(int node, enum node_states state)
{
	return node_isset(node, node_states[state]);
}

static inline void node_set_state(int node, enum node_states state)
{
	__node_set(node, &node_states[state]);
}

static inline void node_clear_state(int node, enum node_states state)
{
	__node_clear(node, &node_states[state]);
}

static inline int num_node_state(enum node_states state)
{
	return nodes_weight(node_states[state]);
}

extern int nr_online_nodes;

#define node_online(node)	node_state((node), N_ONLINE)

#endif /* ! __LINUX_NODEMASK_H__ */
