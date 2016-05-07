#ifndef __ASM_GENERIC_IOCTL_H__
#define __ASM_GENERIC_IOCTL_H__

#include <uapi/asm-generic/ioctl.h>

extern unsigned int __invalid_size_argument_for_IOC;
#define _IOC_TYPECHECK(t) \
	((sizeof(t) == sizeof(t[1]) && \
	  sizeof(t) < (1 << _IOC_SIZEBITS)) ? \
	  sizeof(t) : __invalid_size_argument_for_IOC)

#endif /* ! __ASM_GENERIC_IOCTL_H__ */
