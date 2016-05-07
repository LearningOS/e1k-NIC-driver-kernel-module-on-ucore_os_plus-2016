#ifndef __LINUX_RCULIST_H__
#define __LINUX_RCULIST_H__

/*
 * RCU-protected list version
 */
#include <linux/list.h>
#include <linux/rcupdate.h>

/*
 * INIT_LIST_HEAD_RCU - Initialize a list_head visible to RCU readers
 * @list: list to be initialized
 *
 * You should instead use INIT_LIST_HEAD() for normal initialization and
 * cleanup tasks, when readers have no access to the list being initialized.
 * However, if the list being initialized is visible to readers, you
 * need to keep the compiler from being too mischievous.
 */
static inline void INIT_LIST_HEAD_RCU(struct list_head *list)
{
	ACCESS_ONCE(list->next) = list;
	ACCESS_ONCE(list->prev) = list;
}

/*
 * return the ->next pointer of a list_head in an rcu safe
 * way, we must not access it directly
 */
#define list_next_rcu(list)	(*((struct list_head __rcu **)(&(list)->next)))

static inline void __list_add_rcu(struct list_head *new,
		struct list_head *prev, struct list_head *next)
{
	new->next = next;
	new->prev = prev;
	rcu_assign_pointer(list_next_rcu(prev), new);
	next->prev = new;
}

/**
 * list_entry_rcu - get the struct for this entry
 * @ptr:        the &struct list_head pointer.
 * @type:       the type of the struct this is embedded in.
 * @member:     the name of the list_struct within the struct.
 *
 * This primitive may safely run concurrently with the _rcu list-mutation
 * primitives such as list_add_rcu() as long as it's guarded by rcu_read_lock().
 */
#define list_entry_rcu(ptr, type, member) \
({ \
	typeof(*ptr) __rcu *__ptr = (typeof(*ptr) __rcu __force *)ptr; \
	container_of((typeof(ptr))rcu_dereference_raw(__ptr), type, member); \
})

/*
 * return the first or the next element in an RCU protected hlist
 */
#define hlist_first_rcu(head)	(*((struct hlist_node __rcu **)(&(head)->first)))
#define hlist_next_rcu(node)	(*((struct hlist_node __rcu **)(&(node)->next)))
#define hlist_pprev_rcu(node)	(*((struct hlist_node __rcu **)((node)->pprev)))

#endif /* ! __LINUX_RCULIST_H__ */
