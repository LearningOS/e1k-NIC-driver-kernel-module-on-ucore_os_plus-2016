#ifndef __ASM_GENERIC_CPUTIME_JIFFIES_H__
#define __ASM_GENERIC_CPUTIME_JIFFIES_H__

typedef unsigned long __nocast cputime_t;

/*
 * Convert nanoseconds <-> cputime
 */
#define cputime_to_nsecs(__ct)		\
	jiffies_to_nsecs(cputime_to_jiffies(__ct))

#define nsecs_to_cputime(__nsec)	\
	jiffies_to_cputime(nsecs_to_jiffies(__nsec))

#endif /* ! __ASM_GENERIC_CPUTIME_JIFFIES_H__ */
