#ifndef __UAPI_LINUX_TERMIOS_H__
#define __UAPI_LINUX_TERMIOS_H__

#include <linux/types.h>
#include <asm/termios.h>

#define NFF	5

struct termiox
{
	__u16	x_hflag;
	__u16	x_cflag;
	__u16	x_rflag[NFF];
	__u16	x_sflag;
};

#endif /* ! __UAPI_LINUX_TERMIOS_H__ */
