#ifndef __LINUX_PM_H__
#define __LINUX_PM_H__

#include <linux/list.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/completion.h>

struct device; /* we have a circular dep with device.h */

struct device;

#endif /* ! __LINUX_PM_H__ */
