#ifndef __ASM_GENERIC_BUG_H__
#define __ASM_GENERIC_BUG_H__

#include <linux/compiler.h>

#define BUGFLAG_WARNING		(1 << 0)

#include <linux/kernel.h>

struct bug_entry {
#ifndef CONFIG_GENERIC_BUG_RELATIVE_POINTERS
	unsigned long	bug_addr;
#else
	signed int	bug_addr_disp;
#endif
#ifdef CONFIG_DEBUG_BUGVERBOSE
#ifndef CONFIG_GENERIC_BUG_RELATIVE_POINTERS
	const char	*file;
#else
	signed int	file_disp;
#endif
	unsigned short	line;
#endif
	unsigned short	flags;
};

#ifndef HAVE_ARCH_BUG_ON
#define BUG_ON(condition) do { if (unlikely(condition)) BUG(); } while (0)
#endif

extern __printf(3, 4)
void warn_slowpath_fmt(const char *file, const int line,
		       const char *fmt, ...);

extern void warn_slowpath_null(const char *file, const int line);

#define __WARN()		warn_slowpath_null(__FILE__, __LINE__)
#define __WARN_printf(arg...)	warn_slowpath_fmt(__FILE__, __LINE__, arg)

#ifndef WARN_ON
#define WARN_ON(condition) ({						\
	int __ret_warn_on = !!(condition);				\
	if (unlikely(__ret_warn_on))					\
		__WARN();						\
	unlikely(__ret_warn_on);					\
})
#endif

#ifndef WARN
#define WARN(condition, format...) ({						\
	int __ret_warn_on = !!(condition);				\
	if (unlikely(__ret_warn_on))					\
		__WARN_printf(format);					\
	unlikely(__ret_warn_on);					\
})
#endif

#define WARN_ON_ONCE(condition)	({				\
	static bool __section(.data.unlikely) __warned;		\
	int __ret_warn_once = !!(condition);			\
								\
	if (unlikely(__ret_warn_once))				\
		if (WARN_ON(!__warned)) 			\
			__warned = true;			\
	unlikely(__ret_warn_once);				\
})

#endif /* ! __ASM_GENERIC_BUG_H__ */
