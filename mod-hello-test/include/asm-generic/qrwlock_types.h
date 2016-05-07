#ifndef __ASM_GENERIC_QRWLOCK_TYPES_H__
#define __ASM_GENERIC_QRWLOCK_TYPES_H__

#include <linux/types.h>
#include <asm/spinlock_types.h>

typedef struct qrwlock {
	atomic_t		cnts;
	arch_spinlock_t		lock;
} arch_rwlock_t;

#endif /* ! __ASM_GENERIC_QRWLOCK_TYPES_H__ */
