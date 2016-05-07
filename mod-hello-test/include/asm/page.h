#ifndef __ASM_PAGE_H__
#define __ASM_PAGE_H__

#include <linux/types.h>

#include <asm/page_types.h>

#include <asm/page_64.h>

struct page;

#include <linux/range.h>

#define __pa(x)		__phys_addr((unsigned long)(x))

#define __va(x)			((void *)((unsigned long)(x)+PAGE_OFFSET))

#include <asm-generic/memory_model.h>
#include <asm-generic/getorder.h>

#endif /* ! __ASM_PAGE_H__ */
