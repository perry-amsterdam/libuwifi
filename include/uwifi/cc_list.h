/* Licensed under BSD-MIT - see LICENSE file for details */
#ifndef CCAN_LIST_H
#define CCAN_LIST_H
#if DEBUG
#define CCAN_LIST_DEBUG 1
#endif
#include <stdbool.h>
#include <assert.h>
#include "cc_container_of.h"
#include "cc_check_type.h"

/**
 * stringify - Turn expression into a string literal
 * @expr: any C expression
 *
 * Example:
 *	#define PRINT_COND_IF_FALSE(cond) \
 *		((cond) || printf("%s is false!", stringify(cond)))
 */
#define stringify(expr)		stringify_1(expr)
/* Double-indirection required to stringify expansions */
#define stringify_1(expr)	#expr

/**
 * struct list_node - an entry in a doubly-linked list
 * @next: next entry (self if empty)
 * @prev: previous entry (self if empty)
 *
 * This is used as an entry in a linked list.
 * Example:
 *	struct child {
 *		const char *name;
 *		// Linked list of all us children.
 *		struct list_node list;
 *	};
 */
struct cc_list_node
{
	struct cc_list_node *next, *prev;
};

/**
 * struct list_head - the head of a doubly-linked list
 * @h: the list_head (containing next and prev pointers)
 *
 * This is used as the head of a linked list.
 * Example:
 *	struct parent {
 *		const char *name;
 *		struct list_head children;
 *		unsigned int num_children;
 *	};
 */
struct cc_list_head
{
	struct cc_list_node n;
};

/**
 * list_check - check head of a list for consistency
 * @h: the list_head
 * @abortstr: the location to print on aborting, or NULL.
 *
 * Because list_nodes have redundant information, consistency checking between
 * the back and forward links can be done.  This is useful as a debugging check.
 * If @abortstr is non-NULL, that will be printed in a diagnostic if the list
 * is inconsistent, and the function will abort.
 *
 * Returns the list head if the list is consistent, NULL if not (it
 * can never return NULL if @abortstr is set).
 *
 * See also: list_check_node()
 *
 * Example:
 *	static void dump_parent(struct parent *p)
 *	{
 *		struct child *c;
 *
 *		printf("%s (%u children):\n", p->name, p->num_children);
 *		list_check(&p->children, "bad child list");
 *		list_for_each(&p->children, c, list)
 *			printf(" -> %s\n", c->name);
 *	}
 */
struct cc_list_head *cc_list_check(const struct cc_list_head *h, const char *abortstr);

/**
 * list_check_node - check node of a list for consistency
 * @n: the list_node
 * @abortstr: the location to print on aborting, or NULL.
 *
 * Check consistency of the list node is in (it must be in one).
 *
 * See also: list_check()
 *
 * Example:
 *	static void dump_child(const struct child *c)
 *	{
 *		list_check_node(&c->list, "bad child list");
 *		printf("%s\n", c->name);
 *	}
 */
struct cc_list_node *cc_list_check_node(const struct cc_list_node *n,
				  const char *abortstr);

#define LIST_LOC __FILE__  ":" stringify(__LINE__)
#ifdef CCAN_LIST_DEBUG
#define cc_list_debug(h, loc) cc_list_check((h), loc)
#define cc_list_debug_node(n, loc) cc_list_check_node((n), loc)
#else
#define cc_list_debug(h, loc) (h)
#define cc_list_debug_node(n, loc) (n)
#endif

/**
 * LIST_HEAD_INIT - initializer for an empty list_head
 * @name: the name of the list.
 *
 * Explicit initializer for an empty list.
 *
 * See also:
 *	LIST_HEAD, list_head_init()
 *
 * Example:
 *	static struct list_head my_list = LIST_HEAD_INIT(my_list);
 */
#define CC_LIST_HEAD_INIT(name) { { &name.n, &name.n } }

/**
 * LIST_HEAD - define and initialize an empty list_head
 * @name: the name of the list.
 *
 * The LIST_HEAD macro defines a list_head and initializes it to an empty
 * list.  It can be prepended by "static" to define a static list_head.
 *
 * See also:
 *	LIST_HEAD_INIT, list_head_init()
 *
 * Example:
 *	static LIST_HEAD(my_global_list);
 */
#define CC_LIST_HEAD(name) \
	struct list_head name = CC_LIST_HEAD_INIT(name)

/**
 * list_head_init - initialize a list_head
 * @h: the list_head to set to the empty list
 *
 * Example:
 *	...
 *	struct parent *parent = malloc(sizeof(*parent));
 *
 *	list_head_init(&parent->children);
 *	parent->num_children = 0;
 */
static inline void cc_list_head_init(struct cc_list_head *h)
{
	h->n.next = h->n.prev = &h->n;
}

/**
 * list_add - add an entry at the start of a linked list.
 * @h: the list_head to add the node to
 * @n: the list_node to add to the list.
 *
 * The list_node does not need to be initialized; it will be overwritten.
 * Example:
 *	struct child *child = malloc(sizeof(*child));
 *
 *	child->name = "marvin";
 *	list_add(&parent->children, &child->list);
 *	parent->num_children++;
 */
#define cc_list_add(h, n) cc_list_add_(h, n, LIST_LOC)
static inline void cc_list_add_(struct cc_list_head *h,
			     struct cc_list_node *n,
			     __attribute__((unused)) const char *abortstr)
{
	n->next = h->n.next;
	n->prev = &h->n;
	h->n.next->prev = n;
	h->n.next = n;
	(void)cc_list_debug(h, abortstr);
}

/**
 * list_add_tail - add an entry at the end of a linked list.
 * @h: the list_head to add the node to
 * @n: the list_node to add to the list.
 *
 * The list_node does not need to be initialized; it will be overwritten.
 * Example:
 *	list_add_tail(&parent->children, &child->list);
 *	parent->num_children++;
 */
#define cc_list_add_tail(h, n) cc_list_add_tail_(h, n, LIST_LOC)
static inline void cc_list_add_tail_(struct cc_list_head *h,
				  struct cc_list_node *n,
				  __attribute__((unused)) const char *abortstr)
{
	n->next = &h->n;
	n->prev = h->n.prev;
	h->n.prev->next = n;
	h->n.prev = n;
	(void)cc_list_debug(h, abortstr);
}

/**
 * list_empty - is a list empty?
 * @h: the list_head
 *
 * If the list is empty, returns true.
 *
 * Example:
 *	assert(list_empty(&parent->children) == (parent->num_children == 0));
 */
#define cc_list_empty(h) cc_list_empty_(h, LIST_LOC)
static inline bool cc_list_empty_(const struct cc_list_head *h, __attribute__((unused)) const char* abortstr)
{
	(void)cc_list_debug(h, abortstr);
	return h->n.next == &h->n;
}

/**
 * list_empty_nodebug - is a list empty (and don't perform debug checks)?
 * @h: the list_head
 *
 * If the list is empty, returns true.
 * This differs from list_empty() in that if CCAN_LIST_DEBUG is set it
 * will NOT perform debug checks. Only use this function if you REALLY
 * know what you're doing.
 *
 * Example:
 *	assert(list_empty_nodebug(&parent->children) == (parent->num_children == 0));
 */
#ifndef CCAN_LIST_DEBUG
#define cc_list_empty_nodebug(h) cc_list_empty(h)
#else
static inline bool cc_list_empty_nodebug(const struct cc_list_head *h)
{
	return h->n.next == &h->n;
}
#endif

/**
 * list_del - delete an entry from an (unknown) linked list.
 * @n: the list_node to delete from the list.
 *
 * Note that this leaves @n in an undefined state; it can be added to
 * another list, but not deleted again.
 *
 * See also:
 *	list_del_from()
 *
 * Example:
 *	list_del(&child->list);
 *	parent->num_children--;
 */
#define cc_list_del(n) cc_list_del_(n, LIST_LOC)
static inline void cc_list_del_(struct cc_list_node *n, __attribute__((unused)) const char* abortstr)
{
	(void)cc_list_debug_node(n, abortstr);
	n->next->prev = n->prev;
	n->prev->next = n->next;
#ifdef CCAN_LIST_DEBUG
	/* Catch use-after-del. */
	n->next = n->prev = NULL;
#endif
}

/**
 * list_del_from - delete an entry from a known linked list.
 * @h: the list_head the node is in.
 * @n: the list_node to delete from the list.
 *
 * This explicitly indicates which list a node is expected to be in,
 * which is better documentation and can catch more bugs.
 *
 * See also: list_del()
 *
 * Example:
 *	list_del_from(&parent->children, &child->list);
 *	parent->num_children--;
 */
static inline void cc_list_del_from(struct cc_list_head *h, struct cc_list_node *n)
{
#ifdef CCAN_LIST_DEBUG
	{
		/* Thorough check: make sure it was in list! */
		struct cc_list_node *i;
		for (i = h->n.next; i != n; i = i->next)
			assert(i != &h->n);
	}
#endif /* CCAN_LIST_DEBUG */

	/* Quick test that catches a surprising number of bugs. */
	assert(!cc_list_empty(h));
	cc_list_del(n);
}

/**
 * list_entry - convert a list_node back into the structure containing it.
 * @n: the list_node
 * @type: the type of the entry
 * @member: the list_node member of the type
 *
 * Example:
 *	// First list entry is children.next; convert back to child.
 *	child = list_entry(parent->children.n.next, struct child, list);
 *
 * See Also:
 *	list_top(), list_for_each()
 */
#define cc_list_entry(n, type, member) container_of(n, type, member)

/**
 * list_top - get the first entry in a list
 * @h: the list_head
 * @type: the type of the entry
 * @member: the list_node member of the type
 *
 * If the list is empty, returns NULL.
 *
 * Example:
 *	struct child *first;
 *	first = list_top(&parent->children, struct child, list);
 *	if (!first)
 *		printf("Empty list!\n");
 */
#define cc_list_top(h, type, member)					\
	((type *)cc_list_top_((h), cc_list_off_(type, member)))

static inline const void *cc_list_top_(const struct cc_list_head *h, size_t off)
{
	if (cc_list_empty(h))
		return NULL;
	return (const char *)h->n.next - off;
}

/**
 * list_pop - remove the first entry in a list
 * @h: the list_head
 * @type: the type of the entry
 * @member: the list_node member of the type
 *
 * If the list is empty, returns NULL.
 *
 * Example:
 *	struct child *one;
 *	one = list_pop(&parent->children, struct child, list);
 *	if (!one)
 *		printf("Empty list!\n");
 */
#define cc_list_pop(h, type, member)					\
	((type *)cc_list_pop_((h), cc_list_off_(type, member)))

static inline const void *cc_list_pop_(const struct cc_list_head *h, size_t off)
{
	struct cc_list_node *n;

	if (cc_list_empty(h))
		return NULL;
	n = h->n.next;
	cc_list_del(n);
	return (const char *)n - off;
}

/**
 * list_tail - get the last entry in a list
 * @h: the list_head
 * @type: the type of the entry
 * @member: the list_node member of the type
 *
 * If the list is empty, returns NULL.
 *
 * Example:
 *	struct child *last;
 *	last = list_tail(&parent->children, struct child, list);
 *	if (!last)
 *		printf("Empty list!\n");
 */
#define cc_list_tail(h, type, member) \
	((type *)cc_list_tail_((h), cc_list_off_(type, member)))

static inline const void *cc_list_tail_(const struct cc_list_head *h, size_t off)
{
	if (cc_list_empty(h))
		return NULL;
	return (const char *)h->n.prev - off;
}

/**
 * list_for_each - iterate through a list.
 * @h: the list_head (warning: evaluated multiple times!)
 * @i: the structure containing the list_node
 * @member: the list_node member of the structure
 *
 * This is a convenient wrapper to iterate @i over the entire list.  It's
 * a for loop, so you can break and continue as normal.
 *
 * Example:
 *	list_for_each(&parent->children, child, list)
 *		printf("Name: %s\n", child->name);
 */
#define cc_list_for_each(h, i, member)					\
	cc_list_for_each_off(h, i, cc_list_off_var_(i, member))

/**
 * list_for_each_rev - iterate through a list backwards.
 * @h: the list_head
 * @i: the structure containing the list_node
 * @member: the list_node member of the structure
 *
 * This is a convenient wrapper to iterate @i over the entire list.  It's
 * a for loop, so you can break and continue as normal.
 *
 * Example:
 *	list_for_each_rev(&parent->children, child, list)
 *		printf("Name: %s\n", child->name);
 */
#define cc_list_for_each_rev(h, i, member)					\
	for (i = container_of_var(list_debug(h,	LIST_LOC)->n.prev, i, member); \
	     &i->member != &(h)->n;					\
	     i = container_of_var(i->member.prev, i, member))

/**
 * list_for_each_safe - iterate through a list, maybe during deletion
 * @h: the list_head
 * @i: the structure containing the list_node
 * @nxt: the structure containing the list_node
 * @member: the list_node member of the structure
 *
 * This is a convenient wrapper to iterate @i over the entire list.  It's
 * a for loop, so you can break and continue as normal.  The extra variable
 * @nxt is used to hold the next element, so you can delete @i from the list.
 *
 * Example:
 *	struct child *next;
 *	list_for_each_safe(&parent->children, child, next, list) {
 *		list_del(&child->list);
 *		parent->num_children--;
 *	}
 */
#define cc_list_for_each_safe(h, i, nxt, member)				\
	cc_list_for_each_safe_off(h, i, nxt, cc_list_off_var_(i, member))

/**
 * list_next - get the next entry in a list
 * @h: the list_head
 * @i: a pointer to an entry in the list.
 * @member: the list_node member of the structure
 *
 * If @i was the last entry in the list, returns NULL.
 *
 * Example:
 *	struct child *second;
 *	second = list_next(&parent->children, first, list);
 *	if (!second)
 *		printf("No second child!\n");
 */
#define cc_list_next(h, i, member)						\
	((list_typeof(i))cc_list_entry_or_null(list_debug(h,		\
					    __FILE__ ":" stringify(__LINE__)), \
					    (i)->member.next,		\
					    cc_list_off_var_((i), member)))

/**
 * list_prev - get the previous entry in a list
 * @h: the list_head
 * @i: a pointer to an entry in the list.
 * @member: the list_node member of the structure
 *
 * If @i was the first entry in the list, returns NULL.
 *
 * Example:
 *	first = list_prev(&parent->children, second, list);
 *	if (!first)
 *		printf("Can't go back to first child?!\n");
 */
#define cc_list_prev(h, i, member)						\
	((list_typeof(i))cc_list_entry_or_null(list_debug(h,		\
					    __FILE__ ":" stringify(__LINE__)), \
					    (i)->member.prev,		\
					    cc_list_off_var_((i), member)))

/**
 * list_append_list - empty one list onto the end of another.
 * @to: the list to append into
 * @from: the list to empty.
 *
 * This takes the entire contents of @from and moves it to the end of
 * @to.  After this @from will be empty.
 *
 * Example:
 *	struct list_head adopter;
 *
 *	list_append_list(&adopter, &parent->children);
 *	assert(list_empty(&parent->children));
 *	parent->num_children = 0;
 */
#define cc_list_append_list(t, f) cc_list_append_list_(t, f,			\
				   __FILE__ ":" stringify(__LINE__))
static inline void cc_list_append_list_(struct cc_list_head *to,
				     struct cc_list_head *from,
				     __attribute__((unused)) const char *abortstr)
{
	struct cc_list_node *from_tail = cc_list_debug(from, abortstr)->n.prev;
	struct cc_list_node *to_tail = cc_list_debug(to, abortstr)->n.prev;

	/* Sew in head and entire list. */
	to->n.prev = from_tail;
	from_tail->next = &to->n;
	to_tail->next = &from->n;
	from->n.prev = to_tail;

	/* Now remove head. */
	cc_list_del(&from->n);
	   cc_list_head_init(from);
}

/**
 * list_prepend_list - empty one list into the start of another.
 * @to: the list to prepend into
 * @from: the list to empty.
 *
 * This takes the entire contents of @from and moves it to the start
 * of @to.  After this @from will be empty.
 *
 * Example:
 *	list_prepend_list(&adopter, &parent->children);
 *	assert(list_empty(&parent->children));
 *	parent->num_children = 0;
 */
#define list_prepend_list(t, f) list_prepend_list_(t, f, LIST_LOC)
static inline void cc_list_prepend_list_(struct cc_list_head *to,
				      struct cc_list_head *from,
				      __attribute__((unused)) const char *abortstr)
{
	struct cc_list_node *from_tail = cc_list_debug(from, abortstr)->n.prev;
	struct cc_list_node *to_head = cc_list_debug(to, abortstr)->n.next;

	/* Sew in head and entire list. */
	to->n.next = &from->n;
	from->n.prev = &to->n;
	to_head->prev = from_tail;
	from_tail->next = to_head;

	/* Now remove head. */
	cc_list_del(&from->n);
	   cc_list_head_init(from);
}

/**
 * list_for_each_off - iterate through a list of memory regions.
 * @h: the list_head
 * @i: the pointer to a memory region wich contains list node data.
 * @off: offset(relative to @i) at which list node data resides.
 *
 * This is a low-level wrapper to iterate @i over the entire list, used to
 * implement all oher, more high-level, for-each constructs. It's a for loop,
 * so you can break and continue as normal.
 *
 * WARNING! Being the low-level macro that it is, this wrapper doesn't know
 * nor care about the type of @i. The only assumtion made is that @i points
 * to a chunk of memory that at some @offset, relative to @i, contains a
 * properly filled `struct node_list' which in turn contains pointers to
 * memory chunks and it's turtles all the way down. Whith all that in mind
 * remember that given the wrong pointer/offset couple this macro will
 * happilly churn all you memory untill SEGFAULT stops it, in other words
 * caveat emptor.
 *
 * It is worth mentioning that one of legitimate use-cases for that wrapper
 * is operation on opaque types with known offset for `struct list_node'
 * member(preferably 0), because it allows you not to disclose the type of
 * @i.
 *
 * Example:
 *	list_for_each_off(&parent->children, child,
 *				offsetof(struct child, list))
 *		printf("Name: %s\n", child->name);
 */
#define cc_list_for_each_off(h, i, off)                                    \
	for (i = cc_list_node_to_off_(cc_list_debug(h, LIST_LOC)->n.next,	\
				   (off));				\
       cc_list_node_from_off_((void *)i, (off)) != &(h)->n;                \
       i = cc_list_node_to_off_(cc_list_node_from_off_((void *)i, (off))->next, \
                             (off)))

/**
 * list_for_each_safe_off - iterate through a list of memory regions, maybe
 * during deletion
 * @h: the list_head
 * @i: the pointer to a memory region wich contains list node data.
 * @nxt: the structure containing the list_node
 * @off: offset(relative to @i) at which list node data resides.
 *
 * For details see `list_for_each_off' and `list_for_each_safe'
 * descriptions.
 *
 * Example:
 *	list_for_each_safe_off(&parent->children, child,
 *		next, offsetof(struct child, list))
 *		printf("Name: %s\n", child->name);
 */
#define cc_list_for_each_safe_off(h, i, nxt, off)                          \
	for (i = cc_list_node_to_off_(cc_list_debug(h, LIST_LOC)->n.next,	\
				   (off)),				\
         nxt = cc_list_node_to_off_(cc_list_node_from_off_(i, (off))->next,   \
                                 (off));                                \
       cc_list_node_from_off_(i, (off)) != &(h)->n;                        \
       i = nxt,                                                         \
         nxt = cc_list_node_to_off_(cc_list_node_from_off_(i, (off))->next,   \
                                 (off)))


/* Other -off variants. */
#define cc_list_entry_off(n, type, off)		\
	((type *)cc_list_node_from_off_((n), (off)))

#define cc_list_head_off(h, type, off)		\
	((type *)cc_list_head_off((h), (off)))

#define cc_list_tail_off(h, type, off)		\
	((type *)cc_list_tail_((h), (off)))

#define cc_list_add_off(h, n, off)                 \
	list_add((h), cc_list_node_from_off_((n), (off)))

#define cc_list_del_off(n, off)                    \
	list_del(cc_list_node_from_off_((n), (off)))

#define cc_list_del_from_off(h, n, off)			\
	list_del_from(h, cc_list_node_from_off_((n), (off)))

/* Offset helper functions so we only single-evaluate. */
static inline void *cc_list_node_to_off_(struct cc_list_node *node, size_t off)
{
	return (void *)((char *)node - off);
}
static inline struct cc_list_node *cc_list_node_from_off_(void *ptr, size_t off)
{
	return (struct cc_list_node *)((char *)ptr + off);
}

/* Get the offset of the member, but make sure it's a list_node. */
#define cc_list_off_(type, member)					\
	(container_off(type, member) +				\
	 check_type(((type *)0)->member, struct cc_list_node))

#define cc_list_off_var_(var, member)			\
	(container_off_var(var, member) +		\
	 check_type(var->member, struct cc_list_node))

#define cc_list_typeof(var) typeof(var)

/* Returns member, or NULL if at end of list. */
static inline void *cc_list_entry_or_null(const struct cc_list_head *h,
				       const struct cc_list_node *n,
				       size_t off)
{
	if (n == &h->n)
		return NULL;
	return (char *)n - off;
}
#endif /* CCAN_LIST_H */
