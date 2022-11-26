#ifndef _RBTREE_H_
#define _RBTREE_H_

#include <types.h>
#include <lib.h>

// #define rb_parent(r)   ((struct rb_node *)((r)->__rb_parent_color & ~3))

#define	rb_entry(ptr, type, member) container_of(ptr, type, member)

 
typedef struct _rb_node {
    uint32_t rb_parent_color;
    struct _rb_node *right;
    struct _rb_node *left;
} rb_node;


void add_rbnode(rb_node *node, uint32_t key);
void remove_rbnode(rb_node *node);

#endif /* _RBTREE_H_ */