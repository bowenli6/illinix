#ifndef _RBTREE_H_
#define _RBTREE_H_

#include <types.h>

 
typedef struct rb_node {
    uint32_t rb_parent_color;
    struct rb_node *right;
    struct rb_node *left;
} rb_tree;


void add_rbnode(rb_tree *node, uint32_t key);
void remove_rbnode(rb_tree *node);

#endif /* _RBTREE_H_ */