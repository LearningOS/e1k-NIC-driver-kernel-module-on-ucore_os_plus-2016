#ifndef __LINUX_BITOPS_H__
#define __LINUX_BITOPS_H__

#define _LINUX_BITOPS_H
#include <asm/types.h>

#define BIT(nr)			(1UL << (nr))

#define BITS_PER_BYTE		8
#define BITS_TO_LONGS(nr)	DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

/*
 * Include this here because some architectures need generic_ffs/fls in
 * scope
 */
#include <asm/bitops.h>

static inline unsigned long hweight_long(unsigned long w)
{
	return sizeof(w) == 4 ? hweight32(w) : hweight64(w);
}

static inline unsigned fls_long(unsigned long l)
{
	if (sizeof(l) == 4)
		return fls(l);
	return fls64(l);
}

#endif /* ! __LINUX_BITOPS_H__ */
