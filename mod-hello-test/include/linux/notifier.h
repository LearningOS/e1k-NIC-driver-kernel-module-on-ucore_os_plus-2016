#ifndef __LINUX_NOTIFIER_H__
#define __LINUX_NOTIFIER_H__

#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/rwsem.h>
#include <linux/srcu.h>

#define NOTIFY_OK		0x0001		/* Suits me */
#define NOTIFY_STOP_MASK	0x8000		/* Don't call further */

#endif /* ! __LINUX_NOTIFIER_H__ */
