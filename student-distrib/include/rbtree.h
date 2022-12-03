#ifndef _RBTREE_H_
#define _RBTREE_H_

#include <types.h>
#include <lib.h>

struct _rb_node {
    uint32_t rb_parent_color;               /* store both the address of the parent and the color of the node */
    struct _rb_node *rb_right;               /* right child */
    struct _rb_node *rb_left;                /* left child */
} __attribute__ ((aligned(sizeof(long))));  /* 32-bit aligned so that bit 0 and 1 are always 0 */


struct _rb_root {
    struct _rb_node *rb_node;
};

typedef struct _rb_node rb_node;
typedef struct _rb_root rb_root;


#define RB_BLACK 1
#define RB_RED 0

/* obtain the address of its parent node */
#define rb_parent(r) ((rb_node *)((r)->rb_parent_color & ~3))

/* obtain the color of the node */
#define rb_color(r) ((r)->rb_parent_color & 1)

/* check if the node color is red */
#define rb_is_red(r) (!rb_color(r))

/* check if the node color is black */
#define rb_is_black(r) (rb_color(r))

/* set node color to red */
#define rb_set_red(r) do { (r)->rb_parent_color &= ~1; } while(0)

/* set node color to black */
#define rb_set_black(r) do { (r)->rb_parent_color |= 1; } while (0)


/* set parent address */
static inline void rb_set_parent(rb_node *node, rb_node *parent) {
    node->rb_parent_color = (node->rb_parent_color & 3) | (uint32_t)parent;
}

/* set node color */
static inline void rb_set_color(rb_node *node, int color) {
    node->rb_parent_color = (node->rb_parent_color & ~1) | color;
}



#define RB_ROOT (struct rb_root) { NULL, }
#define RB_EMPTY_ROOT(root) ((root)->rb_node == NULL)
#define RB_EMPTY_NODE(node) (rb_parent(node) == node)
#define RB_CLEAR_NODE(node) (rb_set_parent(node, node))



static inline void rb_init_node(rb_node *rb) {
    rb->rb_parent_color = 0;
    rb->rb_right = NULL;
    rb->rb_left = NULL;
    RB_CLEAR_NODE(rb);
}

static inline void rb_link_node(rb_node *node, rb_node *parent, rb_node **link) {
    /* set parent */
    node->rb_parent_color = (uint32_t)parent;

    /* new node does not have children */
    node->rb_left = node->rb_right = NULL;

    /* assign node to link */
    *link = node;
}

/* get the address of the value entry represented by the red-black tree */
#define rb_entry(node, type, member) container_of(node, type, member);


/* implemented in rbtree.c */

/* insert */
void rb_insert_color(rb_node *node, rb_root *root);

/* remove */
void rb_erase(rb_node *node, rb_root *root);

/* get node */
rb_node *rb_first(const rb_root *root);
rb_node *rb_last(const rb_root *root);
rb_node *rb_next(const rb_node *node);
rb_node *rb_prev(const rb_node *node);

#endif /* _RBTREE_H_ */
