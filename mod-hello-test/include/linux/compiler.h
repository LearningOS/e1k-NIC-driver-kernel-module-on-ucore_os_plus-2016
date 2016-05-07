#ifndef __LINUX_COMPILER_H__
#define __LINUX_COMPILER_H__

#define __LINUX_COMPILER_H

# define __user
# define __kernel

# define __force
# define __nocast
# define __iomem
# define __chk_user_ptr(x) (void)0

# define __acquires(x)
# define __releases(x)
# define __acquire(x) (void)0
# define __release(x) (void)0
# define __cond_lock(x,c) (c)
# define __percpu
# define __rcu

#define ___PASTE(a,b) a##b
#define __PASTE(a,b) ___PASTE(a,b)

#include <linux/compiler-gcc.h>

#define notrace __attribute__((no_instrument_function))

#include <linux/compiler-clang.h>

# define likely(x)	__builtin_expect(!!(x), 1)
# define unlikely(x)	__builtin_expect(!!(x), 0)

#ifndef unreachable
# define unreachable() do { } while (1)
#endif

#ifndef __UNIQUE_ID
# define __UNIQUE_ID(prefix) __PASTE(__PASTE(__UNIQUE_ID_, prefix), __LINE__)
#endif

#undef __deprecated
#undef __deprecated_for_modules
#define __deprecated

#ifndef __cold
#define __cold
#endif

#ifndef __section
# define __section(S) __attribute__ ((__section__(#S)))
#endif

#ifndef __visible
#define __visible
#endif

#ifndef __native_word
# define __native_word(t) (sizeof(t) == sizeof(int) || sizeof(t) == sizeof(long))
#endif

#ifndef __compiletime_warning
# define __compiletime_warning(message)
#endif

# define __compiletime_error(message)

#  define __compiletime_error_fallback(condition) \
	do { ((void)sizeof(char[1 - 2 * condition])); } while (0)

#define __compiletime_assert(condition, msg, prefix, suffix)		\
	do {								\
		bool __cond = !(condition);				\
		extern void prefix ## suffix(void) __compiletime_error(msg); \
		if (__cond)						\
			prefix ## suffix();				\
		__compiletime_error_fallback(__cond);			\
	} while (0)

#define _compiletime_assert(condition, msg, prefix, suffix) \
	__compiletime_assert(condition, msg, prefix, suffix)

/**
 * compiletime_assert - break build and emit msg if condition is false
 * @condition: a compile-time constant condition to check
 * @msg:       a message to emit if condition is false
 *
 * In tradition of POSIX assert, this macro will break the build if the
 * supplied condition is *false*, emitting the supplied error message if the
 * compiler has support to do so.
 */
#define compiletime_assert(condition, msg) \
	_compiletime_assert(condition, msg, __compiletime_assert_, __LINE__)

#define compiletime_assert_atomic_type(t)				\
	compiletime_assert(__native_word(t),				\
		"Need native word sized stores/loads for atomicity.")

/*
 * Prevent the compiler from merging or refetching accesses.  The compiler
 * is also forbidden from reordering successive instances of ACCESS_ONCE(),
 * but only when the compiler is aware of some particular ordering.  One way
 * to make the compiler aware of ordering is to put the two invocations of
 * ACCESS_ONCE() in different C statements.
 *
 * This macro does absolutely -nothing- to prevent the CPU from reordering,
 * merging, or refetching absolutely anything at any time.  Its main intended
 * use is to mediate communication between process-level code and irq/NMI
 * handlers, all running on the same CPU.
 */
#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))

#endif /* ! __LINUX_COMPILER_H__ */
