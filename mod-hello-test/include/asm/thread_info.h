#ifndef __ASM_THREAD_INFO_H__
#define __ASM_THREAD_INFO_H__

#include <linux/compiler.h>
#include <asm/page.h>
#include <asm/percpu.h>
#include <asm/types.h>

struct task_struct;
struct exec_domain;
#include <asm/processor.h>
#include <linux/atomic.h>

struct thread_info {
	struct task_struct	*task;		/* main task structure */
	struct exec_domain	*exec_domain;	/* execution domain */
	__u32			flags;		/* low level flags */
	__u32			status;		/* thread synchronous flags */
	__u32			cpu;		/* current CPU */
	int			saved_preempt_count;
	mm_segment_t		addr_limit;
	struct restart_block    restart_block;
	void __user		*sysenter_return;
	unsigned int		sig_on_uaccess_error:1;
	unsigned int		uaccess_err:1;	/* uaccess failed */
};

#define TIF_SIGPENDING		2	/* signal pending */
#define TIF_NEED_RESCHED	3	/* rescheduling necessary */

#define TIF_SECCOMP		8	/* secure computing */

#define TIF_POLLING_NRFLAG	21	/* idle is polling for TIF_NEED_RESCHED */

#define TIF_ADDR32		29	/* 32-bit address space on 64 bits */

#define KERNEL_STACK_OFFSET	(5*(BITS_PER_LONG/8))

DECLARE_PER_CPU(unsigned long, kernel_stack);

static inline struct thread_info *current_thread_info(void)
{
	struct thread_info *ti;
	ti = (void *)(this_cpu_read_stable(kernel_stack) +
		      KERNEL_STACK_OFFSET - THREAD_SIZE);
	return ti;
}

/*
 * Thread-synchronous status.
 *
 * This is different from the flags in that nobody else
 * ever touches our thread-synchronous status, so we don't
 * have to worry about atomic accesses.
 */
#define TS_COMPAT		0x0002	/* 32bit syscall active (64BIT)*/
#define TS_RESTORE_SIGMASK	0x0008	/* restore signal mask in do_signal() */

#define HAVE_SET_RESTORE_SIGMASK	1

static inline bool test_restore_sigmask(void)
{
	return current_thread_info()->status & TS_RESTORE_SIGMASK;
}
static inline bool test_and_clear_restore_sigmask(void)
{
	struct thread_info *ti = current_thread_info();
	if (!(ti->status & TS_RESTORE_SIGMASK))
		return false;
	ti->status &= ~TS_RESTORE_SIGMASK;
	return true;
}

#endif /* ! __ASM_THREAD_INFO_H__ */
