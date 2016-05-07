#ifndef __UAPI_ASM_GENERIC_POLL_H__
#define __UAPI_ASM_GENERIC_POLL_H__

#define POLLIN		0x0001

#define POLLOUT		0x0004

#define POLLRDNORM	0x0040

#ifndef POLLWRNORM
#define POLLWRNORM	0x0100
#endif

struct pollfd {
	int fd;
	short events;
	short revents;
};

#endif /* ! __UAPI_ASM_GENERIC_POLL_H__ */
