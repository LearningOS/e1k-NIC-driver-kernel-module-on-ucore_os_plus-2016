#ifndef __LINUX_MODULEPARAM_H__
#define __LINUX_MODULEPARAM_H__

#include <linux/init.h>
#include <linux/stringify.h>
#include <linux/kernel.h>

#define MODULE_PARAM_PREFIX KBUILD_MODNAME "."

#define MAX_PARAM_PREFIX_LEN (64 - sizeof(unsigned long))

#define __MODULE_INFO(tag, name, info)					  \
  struct __UNIQUE_ID(name) {}

#define __MODULE_PARM_TYPE(name, _type)					  \
  __MODULE_INFO(parmtype, name##type, #name ":" _type)

struct kernel_param;

struct kernel_param_ops {
	/* How the ops should behave */
	unsigned int flags;
	/* Returns 0, or -errno.  arg is in kp->arg. */
	int (*set)(const char *val, const struct kernel_param *kp);
	/* Returns length written or -errno.  Buffer is 4k (ie. be short!) */
	int (*get)(char *buffer, const struct kernel_param *kp);
	/* Optional function to free kp->arg when module unloaded. */
	void (*free)(void *arg);
};

struct kernel_param {
	const char *name;
	const struct kernel_param_ops *ops;
	u16 perm;
	s16 level;
	union {
		void *arg;
		const struct kparam_string *str;
		const struct kparam_array *arr;
	};
};

struct kparam_string {
	unsigned int maxlen;
	char *string;
};

struct kparam_array
{
	unsigned int max;
	unsigned int elemsize;
	unsigned int *num;
	const struct kernel_param_ops *ops;
	void *elem;
};

/**
 * module_param - typesafe helper for a module/cmdline parameter
 * @value: the variable to alter, and exposed parameter name.
 * @type: the type of the parameter
 * @perm: visibility in sysfs.
 *
 * @value becomes the module parameter, or (prefixed by KBUILD_MODNAME and a
 * ".") the kernel commandline parameter.  Note that - is changed to _, so
 * the user can use "foo-bar=1" even for variable "foo_bar".
 *
 * @perm is 0 if the the variable is not to appear in sysfs, or 0444
 * for world-readable, 0644 for root-writable, etc.  Note that if it
 * is writable, you may need to use kparam_block_sysfs_write() around
 * accesses (esp. charp, which can be kfreed when it changes).
 *
 * The @type is simply pasted to refer to a param_ops_##type and a
 * param_check_##type: for convenience many standard types are provided but
 * you can create your own by defining those variables.
 *
 * Standard types are:
 *	byte, short, ushort, int, uint, long, ulong
 *	charp: a character pointer
 *	bool: a bool, values 0/1, y/n, Y/N.
 *	invbool: the above, only sense-reversed (N = true).
 */
#define module_param(name, type, perm)				\
	module_param_named(name, name, type, perm)

/**
 * module_param_named - typesafe helper for a renamed module/cmdline parameter
 * @name: a valid C identifier which is the parameter name.
 * @value: the actual lvalue to alter.
 * @type: the type of the parameter
 * @perm: visibility in sysfs.
 *
 * Usually it's a good idea to have variable names and user-exposed names the
 * same, but that's harder if the variable must be non-static or is inside a
 * structure.  This allows exposure under a different name.
 */
#define module_param_named(name, value, type, perm)			   \
	param_check_##type(name, &(value));				   \
	module_param_cb(name, &param_ops_##type, &value, perm);		   \
	__MODULE_PARM_TYPE(name, #type)

/**
 * module_param_cb - general callback for a module/cmdline parameter
 * @name: a valid C identifier which is the parameter name.
 * @ops: the set & get operations for this parameter.
 * @perm: visibility in sysfs.
 *
 * The ops can have NULL set or get functions.
 */
#define module_param_cb(name, ops, arg, perm)				      \
	__module_param_call(MODULE_PARAM_PREFIX, name, ops, arg, perm, -1)

#define __moduleparam_const const

/* This is the fundamental function for registering boot/module
   parameters. */
#define __module_param_call(prefix, name, ops, arg, perm, level)	\
	/* Default value instead of permissions? */			\
	static const char __param_str_##name[] = prefix #name; \
	static struct kernel_param __moduleparam_const __param_##name	\
	__used								\
    __attribute__ ((unused,__section__ ("__param"),aligned(sizeof(void *)))) \
	= { __param_str_##name, ops, VERIFY_OCTAL_PERMISSIONS(perm),	\
	    level, { arg } }

/* The macros to do compile-time type checking stolen from Jakub
   Jelinek, who IIRC came up with this idea for the 2.4 module init code. */
#define __param_check(name, p, type) \
	static inline type __always_unused *__check_##name(void) { return(p); }

extern struct kernel_param_ops param_ops_int;

#define param_check_int(name, p) __param_check(name, p, int)

struct module;

#endif /* ! __LINUX_MODULEPARAM_H__ */
