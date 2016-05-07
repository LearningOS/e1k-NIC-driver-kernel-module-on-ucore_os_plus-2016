#ifndef __LINUX_DCACHE_H__
#define __LINUX_DCACHE_H__

#include <linux/atomic.h>
#include <linux/list.h>
#include <linux/rculist.h>
#include <linux/rculist_bl.h>
#include <linux/spinlock.h>
#include <linux/seqlock.h>
#include <linux/cache.h>
#include <linux/rcupdate.h>
#include <linux/lockref.h>

struct nameidata;
struct path;
struct vfsmount;

#define IS_ROOT(x) ((x) == (x)->d_parent)

 #define HASH_LEN_DECLARE u32 hash; u32 len;

/*
 * "quick string" -- eases parameter passing, but more importantly
 * saves "metadata" about the string (ie length and the hash).
 *
 * hash comes first so it snuggles against d_parent in the
 * dentry.
 */
struct qstr {
	union {
		struct {
			HASH_LEN_DECLARE;
		};
		u64 hash_len;
	};
	const unsigned char *name;
};

# define DNAME_INLINE_LEN 32 /* 192 bytes */

#define d_lock	d_lockref.lock

struct dentry {
	/* RCU lookup touched fields */
	unsigned int d_flags;		/* protected by d_lock */
	seqcount_t d_seq;		/* per dentry seqlock */
	struct hlist_bl_node d_hash;	/* lookup hash list */
	struct dentry *d_parent;	/* parent directory */
	struct qstr d_name;
	struct inode *d_inode;		/* Where the name belongs to - NULL is
					 * negative */
	unsigned char d_iname[DNAME_INLINE_LEN];	/* small names */

	/* Ref lookup also touches following */
	struct lockref d_lockref;	/* per-dentry lock and refcount */
	const struct dentry_operations *d_op;
	struct super_block *d_sb;	/* The root of the dentry tree */
	unsigned long d_time;		/* used by d_revalidate */
	void *d_fsdata;			/* fs-specific data */

	struct list_head d_lru;		/* LRU list */
	/*
	 * d_child and d_rcu can share memory
	 */
	union {
		struct list_head d_child;	/* child of parent list */
	 	struct rcu_head d_rcu;
	} d_u;
	struct list_head d_subdirs;	/* our children */
	struct hlist_node d_alias;	/* inode alias list */
};

struct dentry_operations {
	int (*d_revalidate)(struct dentry *, unsigned int);
	int (*d_weak_revalidate)(struct dentry *, unsigned int);
	int (*d_hash)(const struct dentry *, struct qstr *);
	int (*d_compare)(const struct dentry *, const struct dentry *,
			unsigned int, const char *, const struct qstr *);
	int (*d_delete)(const struct dentry *);
	void (*d_release)(struct dentry *);
	void (*d_prune)(struct dentry *);
	void (*d_iput)(struct dentry *, struct inode *);
	char *(*d_dname)(struct dentry *, char *, int);
	struct vfsmount *(*d_automount)(struct path *);
	int (*d_manage)(struct dentry *, bool);
} ____cacheline_aligned;

#define DCACHE_CANT_MOUNT		0x00000100

#define DCACHE_MOUNTED			0x00010000 /* is a mountpoint */
#define DCACHE_NEED_AUTOMOUNT		0x00020000 /* handle automount on this dir */
#define DCACHE_MANAGE_TRANSIT		0x00040000 /* manage transit from this dirent */
#define DCACHE_MANAGED_DENTRY \
	(DCACHE_MOUNTED|DCACHE_NEED_AUTOMOUNT|DCACHE_MANAGE_TRANSIT)

#define DCACHE_ENTRY_TYPE		0x00700000
#define DCACHE_MISS_TYPE		0x00000000 /* Negative dentry */
#define DCACHE_DIRECTORY_TYPE		0x00100000 /* Normal directory */
#define DCACHE_AUTODIR_TYPE		0x00200000 /* Lookupless directory (presumed automount) */
#define DCACHE_SYMLINK_TYPE		0x00300000 /* Symlink */
#define DCACHE_FILE_TYPE		0x00400000 /* Other file type */

/*
 * These are the low-level FS interfaces to the dcache..
 */
extern void d_instantiate(struct dentry *, struct inode *);
extern struct dentry * d_instantiate_unique(struct dentry *, struct inode *);

/*
 * This adds the entry to the hash queues.
 */
extern void d_rehash(struct dentry *);

static inline int d_unhashed(const struct dentry *dentry)
{
	return hlist_bl_unhashed(&dentry->d_hash);
}

/*
 * Directory cache entry type accessor functions.
 */
static inline void __d_set_type(struct dentry *dentry, unsigned type)
{
	dentry->d_flags = (dentry->d_flags & ~DCACHE_ENTRY_TYPE) | type;
}

static inline unsigned __d_entry_type(const struct dentry *dentry)
{
	return dentry->d_flags & DCACHE_ENTRY_TYPE;
}

static inline bool d_can_lookup(const struct dentry *dentry)
{
	return __d_entry_type(dentry) == DCACHE_DIRECTORY_TYPE;
}

static inline bool d_is_autodir(const struct dentry *dentry)
{
	return __d_entry_type(dentry) == DCACHE_AUTODIR_TYPE;
}

static inline bool d_is_negative(const struct dentry *dentry)
{
	return __d_entry_type(dentry) == DCACHE_MISS_TYPE;
}

extern int sysctl_vfs_cache_pressure;

#endif /* ! __LINUX_DCACHE_H__ */
