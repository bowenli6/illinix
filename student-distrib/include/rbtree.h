#ifndef _RBTREE_H_
#define _RBTREE_H_

#include <types.h>
#include <lib.h>

// #define rb_parent(r)   ((struct rb_node *)((r)->__rb_parent_color & ~3))

#define rb_entry(node, type, member) container_of(node, type, member);

 
typedef struct _rb_node {
    uint32_t rb_parent_color;
    struct _rb_node *right;
    struct _rb_node *left;
} rb_node;




#endif /* _RBTREE_H_ */