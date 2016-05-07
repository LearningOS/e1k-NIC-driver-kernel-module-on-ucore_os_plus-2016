#ifndef __UAPI_ASM_GENERIC_SIGNAL_DEFS_H__
#define __UAPI_ASM_GENERIC_SIGNAL_DEFS_H__

#include <linux/compiler.h>

typedef void __signalfn_t(int);
typedef __signalfn_t __user *__sighandler_t;

typedef void __restorefn_t(void);
typedef __restorefn_t __user *__sigrestore_t;

#define SIG_IGN	((__force __sighandler_t)1)	/* ignore signal */

#endif /* ! __UAPI_ASM_GENERIC_SIGNAL_DEFS_H__ */
