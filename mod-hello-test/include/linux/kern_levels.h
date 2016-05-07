#ifndef __LINUX_KERN_LEVELS_H__
#define __LINUX_KERN_LEVELS_H__

#define KERN_SOH	"\001"		/* ASCII Start Of Header */
#define KERN_SOH_ASCII	'\001'

#define KERN_WARNING	KERN_SOH "4"	/* warning conditions */
#define KERN_NOTICE	KERN_SOH "5"	/* normal but significant condition */

#endif /* ! __LINUX_KERN_LEVELS_H__ */
