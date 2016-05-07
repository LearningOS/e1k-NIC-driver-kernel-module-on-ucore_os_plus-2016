#ifndef __ASM_SPINLOCK_TYPES_H__
#define __ASM_SPINLOCK_TYPES_H__

#define _ASM_X86_SPINLOCK_TYPES_H

#include <linux/types.h>

#define __TICKET_LOCK_INC	1
#define TICKET_SLOWPATH_FLAG	((__ticket_t)0)

typedef u8  __ticket_t;
typedef u16 __ticketpair_t;

#define TICKET_LOCK_INC	((__ticket_t)__TICKET_LOCK_INC)

#define TICKET_SHIFT	(sizeof(__ticket_t) * 8)

typedef struct arch_spinlock {
	union {
		__ticketpair_t head_tail;
		struct __raw_tickets {
			__ticket_t head, tail;
		} tickets;
	};
} arch_spinlock_t;

#define __ARCH_SPIN_LOCK_UNLOCKED	{ { 0 } }

#include <asm-generic/qrwlock_types.h>

#endif /* ! __ASM_SPINLOCK_TYPES_H__ */
