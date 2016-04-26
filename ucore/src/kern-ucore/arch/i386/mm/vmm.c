#include <vmm.h>
#include <string.h>
#include <trap.h>
#include <proc.h>
#include <error.h>

static inline void print_pgfault(struct trapframe *tf)
{
	/* error_code:
	 * bit 0 == 0 means no page found, 1 means protection fault
	 * bit 1 == 0 means read, 1 means write
	 * bit 2 == 0 means kernel, 1 means user
	 * */
	kprintf("page fault at 0x%08x: %c/%c [%s].\n", rcr2(),
		(tf->tf_err & 4) ? 'U' : 'K',
		(tf->tf_err & 2) ? 'W' : 'R',
		(tf->tf_err & 1) ? "protection fault" : "no page found");
}

static int pgfault_handler(struct trapframe *tf)
{
	extern struct mm_struct *check_mm_struct;
	struct mm_struct *mm;
	if (check_mm_struct != NULL) {
		assert(current == idleproc);
		mm = check_mm_struct;
	} else {
		if (current == NULL) {
			print_trapframe(tf);
			print_pgfault(tf);
			panic("unhandled page fault.\n");
		}
		mm = current->mm;
	}
	return do_pgfault(mm, tf->tf_err, rcr2());
}

int pgfault_handler_wrapper(int irq, void* data) {
  extern struct trapframe* irq_tf;
  int ret;
  if ((ret = pgfault_handler(irq_tf)) != 0) {
    print_trapframe(irq_tf);
    if (current == NULL) {
      panic("handle pgfault failed. %e\n", ret);
    } else {
      if (trap_in_kernel(irq_tf)) {
        panic("handle pgfault failed in kernel mode. %e\n", ret);
      }
      kprintf("killed by kernel.\n");
      do_exit(-E_KILLED);
    }
  }
  return ret;
}

void vmm_init_pgflt(void)
{
  register_irq(T_PGFLT, pgfault_handler_wrapper, NULL);
}

bool
copy_from_user(struct mm_struct *mm, void *dst, const void *src, size_t len,
	       bool writable)
{
	if (!user_mem_check(mm, (uintptr_t) src, len, writable)) {
		return 0;
	}
	memcpy(dst, src, len);
	return 1;
}

bool copy_to_user(struct mm_struct * mm, void *dst, const void *src, size_t len)
{
	if (!user_mem_check(mm, (uintptr_t) dst, len, 1)) {
		return 0;
	}
	memcpy(dst, src, len);
	return 1;
}

bool copy_string(struct mm_struct * mm, char *dst, const char *src, size_t maxn)
{
	size_t alen, part =
	    ROUNDDOWN((uintptr_t) src + PGSIZE, PGSIZE) - (uintptr_t) src;
	while (1) {
		if (part > maxn) {
			part = maxn;
		}
		if (!user_mem_check(mm, (uintptr_t) src, part, 0)) {
			return 0;
		}
		if ((alen = strnlen(src, part)) < part) {
			memcpy(dst, src, alen + 1);
			return 1;
		}
		if (part == maxn) {
			return 0;
		}
		memcpy(dst, src, part);
		dst += part, src += part, maxn -= part;
		part = PGSIZE;
	}
}
