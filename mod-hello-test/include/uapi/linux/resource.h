#ifndef __UAPI_LINUX_RESOURCE_H__
#define __UAPI_LINUX_RESOURCE_H__

#include <linux/time.h>
#include <linux/types.h>

struct rlimit {
	__kernel_ulong_t	rlim_cur;
	__kernel_ulong_t	rlim_max;
};

/*
 * Due to binary compatibility, the actual resource numbers
 * may be different for different linux versions..
 */
#include <asm/resource.h>

#endif /* ! __UAPI_LINUX_RESOURCE_H__ */
