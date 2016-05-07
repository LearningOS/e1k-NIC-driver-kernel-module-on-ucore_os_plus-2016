#ifndef __LINUX_CDEV_H__
#define __LINUX_CDEV_H__

#include <linux/kobject.h>
#include <linux/kdev_t.h>
#include <linux/list.h>

struct file_operations;
struct inode;
struct module;

struct cdev {
	struct kobject kobj;
	struct module *owner;
	const struct file_operations *ops;
	struct list_head list;
	dev_t dev;
	unsigned int count;
};

void cdev_init(struct cdev *, const struct file_operations *);

int cdev_add(struct cdev *, dev_t, unsigned);

void cdev_del(struct cdev *);

#endif /* ! __LINUX_CDEV_H__ */
