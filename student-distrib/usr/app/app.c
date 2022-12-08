#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* cast the datatype of ptr to type */
#define container_of(ptr, type, member) ({		    \
	void *__mptr = (void *)(ptr);					\
	((type *)(__mptr - offsetof(type, member))); })


/* find the offset of the member within the struct type */
#define offsetof(type, member) \
    ((unsigned int)((unsigned char*)&((type*)0)->member - (unsigned char*)0))


typedef struct _list {
    struct _list *next;
    struct _list *prev;
} list_head;


struct value {
    int a;
    int b;
    list_head node;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }


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




int main(void) {
   printf("L0\n");
    if (fork() != 0) {
        printf("L1\n");
        if (fork() != 0) {
            printf("L2\n");
        }
    }
    printf("Bye\n");
}   
