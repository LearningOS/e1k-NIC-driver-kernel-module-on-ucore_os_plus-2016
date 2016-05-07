#ifndef __LINUX_IOPORT_H__
#define __LINUX_IOPORT_H__

#include <linux/compiler.h>
#include <linux/types.h>
/*
 * Resources are tree-like, allowing
 * nesting etc..
 */
struct resource {
	resource_size_t start;
	resource_size_t end;
	const char *name;
	unsigned long flags;
	struct resource *parent, *sibling, *child;
};

#define IORESOURCE_TYPE_BITS	0x00001f00	/* Resource type */

#define IORESOURCE_UNSET	0x20000000	/* No address assigned yet */

extern struct resource ioport_resource;

static inline unsigned long resource_type(const struct resource *res)
{
	return res->flags & IORESOURCE_TYPE_BITS;
}

extern int __check_region(struct resource *, resource_size_t, resource_size_t);

struct device;

#endif /* ! __LINUX_IOPORT_H__ */
