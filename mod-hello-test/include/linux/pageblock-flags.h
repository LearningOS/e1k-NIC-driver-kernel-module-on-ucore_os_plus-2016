#ifndef __LINUX_PAGEBLOCK_FLAGS_H__
#define __LINUX_PAGEBLOCK_FLAGS_H__

#include <linux/types.h>

struct page;

unsigned long get_pfnblock_flags_mask(struct page *page,
				unsigned long pfn,
				unsigned long end_bitidx,
				unsigned long mask);

#endif /* ! __LINUX_PAGEBLOCK_FLAGS_H__ */
