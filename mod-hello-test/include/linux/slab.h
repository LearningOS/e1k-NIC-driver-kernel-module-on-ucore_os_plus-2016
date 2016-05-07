#ifndef __LINUX_SLAB_H__
#define __LINUX_SLAB_H__

#include <linux/gfp.h>
#include <linux/types.h>
#include <linux/workqueue.h>

/*
 * ZERO_SIZE_PTR will be returned for zero sized kmalloc requests.
 *
 * Dereferencing ZERO_SIZE_PTR will lead to a distinct access fault.
 *
 * ZERO_SIZE_PTR can be passed to kfree though in the same way that NULL can.
 * Both make kfree a no-op.
 */
#define ZERO_SIZE_PTR ((void *)16)

#include <linux/kmemleak.h>

void kfree(const void *);

/*
 * SLUB directly allocates requests fitting in to an order-1 page
 * (PAGE_SIZE*2).  Larger requests are passed to the page allocator.
 */
#define KMALLOC_SHIFT_HIGH	(PAGE_SHIFT + 1)

#ifndef KMALLOC_SHIFT_LOW
#define KMALLOC_SHIFT_LOW	3
#endif

#define KMALLOC_MAX_CACHE_SIZE	(1UL << KMALLOC_SHIFT_HIGH)

/*
 * Kmalloc subsystem.
 */
#ifndef KMALLOC_MIN_SIZE
#define KMALLOC_MIN_SIZE (1 << KMALLOC_SHIFT_LOW)
#endif

extern struct kmem_cache *kmalloc_caches[KMALLOC_SHIFT_HIGH + 1];

/*
 * Figure out which kmalloc slab an allocation of a certain size
 * belongs to.
 * 0 = zero alloc
 * 1 =  65 .. 96 bytes
 * 2 = 120 .. 192 bytes
 * n = 2^(n-1) .. 2^n -1
 */
static __always_inline int kmalloc_index(size_t size)
{
	if (!size)
		return 0;

	if (size <= KMALLOC_MIN_SIZE)
		return KMALLOC_SHIFT_LOW;

	if (KMALLOC_MIN_SIZE <= 32 && size > 64 && size <= 96)
		return 1;
	if (KMALLOC_MIN_SIZE <= 64 && size > 128 && size <= 192)
		return 2;
	if (size <=          8) return 3;
	if (size <=         16) return 4;
	if (size <=         32) return 5;
	if (size <=         64) return 6;
	if (size <=        128) return 7;
	if (size <=        256) return 8;
	if (size <=        512) return 9;
	if (size <=       1024) return 10;
	if (size <=   2 * 1024) return 11;
	if (size <=   4 * 1024) return 12;
	if (size <=   8 * 1024) return 13;
	if (size <=  16 * 1024) return 14;
	if (size <=  32 * 1024) return 15;
	if (size <=  64 * 1024) return 16;
	if (size <= 128 * 1024) return 17;
	if (size <= 256 * 1024) return 18;
	if (size <= 512 * 1024) return 19;
	if (size <= 1024 * 1024) return 20;
	if (size <=  2 * 1024 * 1024) return 21;
	if (size <=  4 * 1024 * 1024) return 22;
	if (size <=  8 * 1024 * 1024) return 23;
	if (size <=  16 * 1024 * 1024) return 24;
	if (size <=  32 * 1024 * 1024) return 25;
	if (size <=  64 * 1024 * 1024) return 26;
	BUG();

	/* Will never be reached. Needed because the compiler may complain */
	return -1;
}

void *__kmalloc(size_t size, gfp_t flags);
void *kmem_cache_alloc(struct kmem_cache *, gfp_t flags);

void *__kmalloc_node(size_t size, gfp_t flags, int node);

extern void *kmem_cache_alloc_trace(struct kmem_cache *, gfp_t, size_t);

extern void *kmem_cache_alloc_node_trace(struct kmem_cache *s,
					   gfp_t gfpflags,
					   int node, size_t size);

#include <linux/slub_def.h>

extern void *kmalloc_order_trace(size_t size, gfp_t flags, unsigned int order);

static __always_inline void *kmalloc_large(size_t size, gfp_t flags)
{
	unsigned int order = get_order(size);
	return kmalloc_order_trace(size, flags, order);
}

/**
 * kmalloc - allocate memory
 * @size: how many bytes of memory are required.
 * @flags: the type of memory to allocate.
 *
 * kmalloc is the normal method of allocating memory
 * for objects smaller than page size in the kernel.
 *
 * The @flags argument may be one of:
 *
 * %GFP_USER - Allocate memory on behalf of user.  May sleep.
 *
 * %GFP_KERNEL - Allocate normal kernel ram.  May sleep.
 *
 * %GFP_ATOMIC - Allocation will not sleep.  May use emergency pools.
 *   For example, use this inside interrupt handlers.
 *
 * %GFP_HIGHUSER - Allocate pages from high memory.
 *
 * %GFP_NOIO - Do not do any I/O at all while trying to get memory.
 *
 * %GFP_NOFS - Do not make any fs calls while trying to get memory.
 *
 * %GFP_NOWAIT - Allocation will not sleep.
 *
 * %__GFP_THISNODE - Allocate node-local memory only.
 *
 * %GFP_DMA - Allocation suitable for DMA.
 *   Should only be used for kmalloc() caches. Otherwise, use a
 *   slab created with SLAB_DMA.
 *
 * Also it is possible to set different flags by OR'ing
 * in one or more of the following additional @flags:
 *
 * %__GFP_COLD - Request cache-cold pages instead of
 *   trying to return cache-warm pages.
 *
 * %__GFP_HIGH - This allocation has high priority and may use emergency pools.
 *
 * %__GFP_NOFAIL - Indicate that this allocation is in no way allowed to fail
 *   (think twice before using).
 *
 * %__GFP_NORETRY - If memory is not immediately available,
 *   then give up at once.
 *
 * %__GFP_NOWARN - If allocation fails, don't issue any warnings.
 *
 * %__GFP_REPEAT - If allocation fails initially, try once more before failing.
 *
 * There are other flags available as well, but these are not intended
 * for general use, and so are not documented here. For a full list of
 * potential flags, always refer to linux/gfp.h.
 */
static __always_inline void *kmalloc(size_t size, gfp_t flags)
{
	if (__builtin_constant_p(size)) {
		if (size > KMALLOC_MAX_CACHE_SIZE)
			return kmalloc_large(size, flags);
#ifndef CONFIG_SLOB
		if (!(flags & GFP_DMA)) {
			int index = kmalloc_index(size);

			if (!index)
				return ZERO_SIZE_PTR;

			return kmem_cache_alloc_trace(kmalloc_caches[index],
					flags, size);
		}
#endif
	}
	return __kmalloc(size, flags);
}

static __always_inline void *kmalloc_node(size_t size, gfp_t flags, int node)
{
#ifndef CONFIG_SLOB
	if (__builtin_constant_p(size) &&
		size <= KMALLOC_MAX_CACHE_SIZE && !(flags & GFP_DMA)) {
		int i = kmalloc_index(size);

		if (!i)
			return ZERO_SIZE_PTR;

		return kmem_cache_alloc_node_trace(kmalloc_caches[i],
						flags, node, size);
	}
#endif
	return __kmalloc_node(size, flags, node);
}

struct seq_file;

/**
 * kmalloc_array - allocate memory for an array.
 * @n: number of elements.
 * @size: element size.
 * @flags: the type of memory to allocate (see kmalloc).
 */
static inline void *kmalloc_array(size_t n, size_t size, gfp_t flags)
{
	if (size != 0 && n > SIZE_MAX / size)
		return NULL;
	return __kmalloc(n * size, flags);
}

#endif /* ! __LINUX_SLAB_H__ */
