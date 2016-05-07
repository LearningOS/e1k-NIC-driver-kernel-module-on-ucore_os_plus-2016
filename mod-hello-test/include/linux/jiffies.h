#ifndef __LINUX_JIFFIES_H__
#define __LINUX_JIFFIES_H__

#include <linux/math64.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/time.h>
#include <linux/timex.h>
#include <asm/param.h>			/* for HZ */

# define SHIFT_HZ	10

#define TICK_NSEC ((NSEC_PER_SEC+HZ/2)/HZ)

/* some arch's have a small-data section that can be accessed register-relative
 * but that can only take up to, say, 4-byte variables. jiffies being part of
 * an 8-byte variable may not be correctly accessed unless we force the issue
 */
#define __jiffy_data  __attribute__((section(".data")))

extern unsigned long volatile __jiffy_data jiffies;

#define SEC_JIFFIE_SC (31 - SHIFT_HZ)

#undef SEC_JIFFIE_SC

extern unsigned int jiffies_to_usecs(const unsigned long j);

extern clock_t jiffies_to_clock_t(unsigned long x);

#endif /* ! __LINUX_JIFFIES_H__ */
