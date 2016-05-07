#ifndef __LINUX_JUMP_LABEL_H__
#define __LINUX_JUMP_LABEL_H__

#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/bug.h>

extern bool static_key_initialized;

#define STATIC_KEY_CHECK_USE() WARN(!static_key_initialized,		      \
				    "%s used before call to jump_label_init", \
				    __func__)

struct static_key {
	atomic_t enabled;
};

struct module;

#include <linux/atomic.h>

static inline int static_key_count(struct static_key *key)
{
	return atomic_read(&key->enabled);
}

static __always_inline bool static_key_false(struct static_key *key)
{
	if (unlikely(static_key_count(key) > 0))
		return true;
	return false;
}

#endif /* ! __LINUX_JUMP_LABEL_H__ */
