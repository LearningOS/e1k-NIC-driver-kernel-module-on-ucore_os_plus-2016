#ifndef __UAPI_ASM_GENERIC_FCNTL_H__
#define __UAPI_ASM_GENERIC_FCNTL_H__

#include <linux/types.h>

#define O_ACCMODE	00000003

#define O_WRONLY	00000001

#ifndef O_NONBLOCK
#define O_NONBLOCK	00004000
#endif
#ifndef O_DSYNC
#define O_DSYNC		00010000	/* used to be O_SYNC, see below */
#endif

#ifndef O_NOATIME
#define O_NOATIME	01000000
#endif

#define __O_SYNC	04000000

#ifndef __ARCH_FLOCK_PAD
#define __ARCH_FLOCK_PAD
#endif

#ifndef __ARCH_FLOCK64_PAD
#define __ARCH_FLOCK64_PAD
#endif

#endif /* ! __UAPI_ASM_GENERIC_FCNTL_H__ */
