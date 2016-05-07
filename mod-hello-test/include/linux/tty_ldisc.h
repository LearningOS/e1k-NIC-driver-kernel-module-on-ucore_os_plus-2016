#ifndef __LINUX_TTY_LDISC_H__
#define __LINUX_TTY_LDISC_H__

#include <linux/fs.h>
#include <linux/wait.h>

/*
 * the semaphore definition
 */
struct ld_semaphore {
	long			count;
	raw_spinlock_t		wait_lock;
	unsigned int		wait_readers;
	struct list_head	read_wait;
	struct list_head	write_wait;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map	dep_map;
#endif
};

struct tty_ldisc_ops {
	int	magic;
	char	*name;
	int	num;
	int	flags;

	/*
	 * The following routines are called from above.
	 */
	int	(*open)(struct tty_struct *);
	void	(*close)(struct tty_struct *);
	void	(*flush_buffer)(struct tty_struct *tty);
	ssize_t	(*chars_in_buffer)(struct tty_struct *tty);
	ssize_t	(*read)(struct tty_struct *tty, struct file *file,
			unsigned char __user *buf, size_t nr);
	ssize_t	(*write)(struct tty_struct *tty, struct file *file,
			 const unsigned char *buf, size_t nr);
	int	(*ioctl)(struct tty_struct *tty, struct file *file,
			 unsigned int cmd, unsigned long arg);
	long	(*compat_ioctl)(struct tty_struct *tty, struct file *file,
				unsigned int cmd, unsigned long arg);
	void	(*set_termios)(struct tty_struct *tty, struct ktermios *old);
	unsigned int (*poll)(struct tty_struct *, struct file *,
			     struct poll_table_struct *);
	int	(*hangup)(struct tty_struct *tty);

	/*
	 * The following routines are called from below.
	 */
	void	(*receive_buf)(struct tty_struct *, const unsigned char *cp,
			       char *fp, int count);
	void	(*write_wakeup)(struct tty_struct *);
	void	(*dcd_change)(struct tty_struct *, unsigned int);
	void	(*fasync)(struct tty_struct *tty, int on);
	int	(*receive_buf2)(struct tty_struct *, const unsigned char *cp,
				char *fp, int count);

	struct  module *owner;

	int refcount;
};

struct tty_ldisc {
	struct tty_ldisc_ops *ops;
	struct tty_struct *tty;
};

#endif /* ! __LINUX_TTY_LDISC_H__ */
