#ifndef __LINUX_SYSFS_H__
#define __LINUX_SYSFS_H__

#include <linux/kernfs.h>
#include <linux/compiler.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/lockdep.h>
#include <linux/kobject_ns.h>
#include <linux/stat.h>
#include <linux/atomic.h>

struct kobject;
struct module;

struct attribute {
	const char		*name;
	umode_t			mode;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	bool			ignore_lockdep:1;
	struct lock_class_key	*key;
	struct lock_class_key	skey;
#endif
};

struct file;
struct vm_area_struct;

struct sysfs_ops {
	ssize_t	(*show)(struct kobject *, struct attribute *, char *);
	ssize_t	(*store)(struct kobject *, struct attribute *, const char *, size_t);
};

int __must_check sysfs_create_file_ns(struct kobject *kobj,
				      const struct attribute *attr,
				      const void *ns);

void sysfs_remove_file_ns(struct kobject *kobj, const struct attribute *attr,
			  const void *ns);

int sysfs_rename_link_ns(struct kobject *kobj, struct kobject *target,
			 const char *old_name, const char *new_name,
			 const void *new_ns);

#endif /* ! __LINUX_SYSFS_H__ */
