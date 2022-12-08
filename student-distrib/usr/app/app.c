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

int8_t* strrev(int8_t* s) {
    register int8_t tmp;
    register int32_t beg = 0;
    register int32_t end = strlen(s) - 1;

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
    }
    return s;
}

typedef unsigned long uint32_t;


int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix) {
    static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int8_t *newbuf = buf;
    int32_t i;
    uint32_t newval = value;

    /* Special case for zero */
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    /* Go through the number one place value at a time, and add the
     * correct digit to "newbuf".  We actually add characters to the
     * ASCII string from lowest place value to highest, which is the
     * opposite of how the number should be printed.  We'll reverse the
     * characters later. */
    while (newval > 0) {
        i = newval % radix;
        *newbuf = lookup[i];
        newbuf++;
        newval /= radix;
    }

    /* Add a terminating NULL */
    *newbuf = '\0';

    /* Reverse the string and return */
    return strrev(buf);
}

int main(void) {
    char buf[100];
   printf("%s\n", itoa(2131231231, buf, 10));
}   
