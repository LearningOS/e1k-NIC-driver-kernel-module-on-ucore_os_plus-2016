#ifndef __ASM_QRWLOCK_H__
#define __ASM_QRWLOCK_H__

#include <asm-generic/qrwlock_types.h>

#define queue_write_unlock queue_write_unlock
static inline void queue_write_unlock(struct qrwlock *lock)
{
        barrier();
        ACCESS_ONCE(*(u8 *)&lock->cnts) = 0;
}

#include <asm-generic/qrwlock.h>

#endif /* ! __ASM_QRWLOCK_H__ */
