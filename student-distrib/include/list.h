#ifndef _LIST_H_
#define _LIST_H_

#include <lib.h>

typedef struct _list {
    struct _list *next;
    struct _list *prev;
} list_head;


#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    list_head name = LIST_HEAD_INIT(name)

static inline void __list_add(list_head *new,
                list_head *prev,
                list_head *next) {
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}


static inline void __list_del(list_head * prev, list_head * next) {
    next->prev = prev;
    prev->next = next;
}


static inline void list_add(list_head *new, list_head *head) {
    __list_add(new, head, head->next);
}


static inline void list_add_tail(list_head *new, list_head *head) {
    __list_add(new, head->prev, head);
}


static inline void list_del(list_head *entry) {
    __list_del(entry->prev, entry->next);
}

static inline int32_t list_empty(list_head *head) {
    return head->next == head;
}

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define list_entry(ptr, type, member) container_of(ptr, type, member)


#endif /* _LIST_H_ */
