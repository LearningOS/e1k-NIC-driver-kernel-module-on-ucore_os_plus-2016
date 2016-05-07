#ifndef __UAPI_ASM_GENERIC_TERMBITS_H__
#define __UAPI_ASM_GENERIC_TERMBITS_H__

#include <linux/posix_types.h>

typedef unsigned char	cc_t;
typedef unsigned int	speed_t;
typedef unsigned int	tcflag_t;

#define NCCS 19

struct ktermios {
	tcflag_t c_iflag;		/* input mode flags */
	tcflag_t c_oflag;		/* output mode flags */
	tcflag_t c_cflag;		/* control mode flags */
	tcflag_t c_lflag;		/* local mode flags */
	cc_t c_line;			/* line discipline */
	cc_t c_cc[NCCS];		/* control characters */
	speed_t c_ispeed;		/* input speed */
	speed_t c_ospeed;		/* output speed */
};

#endif /* ! __UAPI_ASM_GENERIC_TERMBITS_H__ */
