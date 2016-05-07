#ifndef __LINUX_TTY_H__
#define __LINUX_TTY_H__

#include <linux/fs.h>
#include <linux/major.h>
#include <linux/termios.h>
#include <linux/workqueue.h>
#include <linux/tty_driver.h>
#include <linux/tty_ldisc.h>
#include <linux/mutex.h>
#include <linux/tty_flags.h>
#include <uapi/linux/tty.h>
#include <linux/rwsem.h>
#include <linux/llist.h>

struct tty_buffer {
	union {
		struct tty_buffer *next;
		struct llist_node free;
	};
	int used;
	int size;
	int commit;
	int read;
	int flags;
	/* Data points here */
	unsigned long data[0];
};

static inline unsigned char *char_buf_ptr(struct tty_buffer *b, int ofs)
{
	return ((unsigned char *)b->data) + ofs;
}

struct tty_bufhead {
	struct tty_buffer *head;	/* Queue head */
	struct work_struct work;
	struct mutex	   lock;
	atomic_t	   priority;
	struct tty_buffer sentinel;
	struct llist_head free;		/* Free queue head */
	atomic_t	   mem_used;    /* In-use buffers excluding free list */
	int		   mem_limit;
	struct tty_buffer *tail;	/* Active buffer */
};

struct device;
struct signal_struct;

struct tty_port;

struct tty_port_operations {
	/* Return 1 if the carrier is raised */
	int (*carrier_raised)(struct tty_port *port);
	/* Control the DTR line */
	void (*dtr_rts)(struct tty_port *port, int raise);
	/* Called when the last close completes or a hangup finishes
	   IFF the port was initialized. Do not use to free resources. Called
	   under the port mutex to serialize against activate/shutdowns */
	void (*shutdown)(struct tty_port *port);
	/* Called under the port mutex from tty_port_open, serialized using
	   the port mutex */
        /* FIXME: long term getting the tty argument *out* of this would be
           good for consoles */
	int (*activate)(struct tty_port *port, struct tty_struct *tty);
	/* Called on the final put of a port */
	void (*destruct)(struct tty_port *port);
};

struct tty_port {
	struct tty_bufhead	buf;		/* Locked internally */
	struct tty_struct	*tty;		/* Back pointer */
	struct tty_struct	*itty;		/* internal back ptr */
	const struct tty_port_operations *ops;	/* Port operations */
	spinlock_t		lock;		/* Lock protecting tty field */
	int			blocked_open;	/* Waiting to open */
	int			count;		/* Usage count */
	wait_queue_head_t	open_wait;	/* Open waiters */
	wait_queue_head_t	close_wait;	/* Close waiters */
	wait_queue_head_t	delta_msr_wait;	/* Modem status change */
	unsigned long		flags;		/* TTY flags ASY_*/
	unsigned char		console:1,	/* port is a console */
				low_latency:1;	/* optional: tune for latency */
	struct mutex		mutex;		/* Locking */
	struct mutex		buf_mutex;	/* Buffer alloc lock */
	unsigned char		*xmit_buf;	/* Optional buffer */
	unsigned int		close_delay;	/* Close port delay */
	unsigned int		closing_wait;	/* Delay for output */
	int			drain_delay;	/* Set to zero if no pure time
						   based drain is needed else
						   set to size of fifo */
	struct kref		kref;		/* Ref counter */
};

struct tty_operations;

struct tty_struct {
	int	magic;
	struct kref kref;
	struct device *dev;
	struct tty_driver *driver;
	const struct tty_operations *ops;
	int index;

	/* Protects ldisc changes: Lock tty not pty */
	struct ld_semaphore ldisc_sem;
	struct tty_ldisc *ldisc;

	struct mutex atomic_write_lock;
	struct mutex legacy_mutex;
	struct mutex throttle_mutex;
	struct rw_semaphore termios_rwsem;
	struct mutex winsize_mutex;
	spinlock_t ctrl_lock;
	/* Termios values are protected by the termios rwsem */
	struct ktermios termios, termios_locked;
	struct termiox *termiox;	/* May be NULL for unsupported */
	char name[64];
	struct pid *pgrp;		/* Protected by ctrl lock */
	struct pid *session;
	unsigned long flags;
	int count;
	struct winsize winsize;		/* winsize_mutex */
	unsigned char stopped:1, hw_stopped:1, flow_stopped:1, packet:1;
	unsigned char ctrl_status;	/* ctrl_lock */
	unsigned int receive_room;	/* Bytes free for queue */
	int flow_change;

	struct tty_struct *link;
	struct fasync_struct *fasync;
	int alt_speed;		/* For magic substitution of 38400 bps */
	wait_queue_head_t write_wait;
	wait_queue_head_t read_wait;
	struct work_struct hangup_work;
	void *disc_data;
	void *driver_data;
	struct list_head tty_files;

#define N_TTY_BUF_SIZE 4096

	unsigned char closing:1;
	unsigned char *write_buf;
	int write_cnt;
	/* If the tty has a pending do_SAK, queue it here - akpm */
	struct work_struct SAK_work;
	struct tty_port *port;
};

extern dev_t tty_devnum(struct tty_struct *tty);

extern void tty_wait_until_sent(struct tty_struct *tty, long timeout);

extern speed_t tty_termios_baud_rate(struct ktermios *termios);

extern void __lockfunc tty_lock(struct tty_struct *tty);
extern void __lockfunc tty_unlock(struct tty_struct *tty);

#endif /* ! __LINUX_TTY_H__ */
