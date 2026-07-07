# Red-Black Tree (rbtree)

Red-Black Tree, self-balancing BST following Linux kernel style. Supports plain and cached roots (O(1) min lookup), and post-order traversal for safe deletion.

## Data Structures

```c
struct rb_node {
    rb_color_t __rb_parent_color;
    struct rb_node * rb_right;
    struct rb_node * rb_left;
};

struct rb_root {
    struct rb_node * rb_node;
};

struct rb_root_cached {
    struct rb_root rb_root;
    struct rb_node * rb_leftmost;
};
```

## Key Macros

```c
#define rb_parent(r)                ((struct rb_node *)((r)->__rb_parent_color & ~3))
#define RB_ROOT                     (struct rb_root) { NULL, }
#define RB_ROOT_CACHED              (struct rb_root_cached) { {NULL, }, NULL }
#define RB_EMPTY_ROOT(root)         ((root)->rb_node == NULL)
#define RB_EMPTY_NODE(node)         ((node)->__rb_parent_color == (rb_color_t)(node))
#define RB_CLEAR_NODE(node)         ((node)->__rb_parent_color = (rb_color_t)(node))
#define rb_entry(ptr, type, member) container_of(ptr, type, member)
#define rb_first_cached(root)       (root)->rb_leftmost
```

## API

```c
void rb_link_node(struct rb_node * node, struct rb_node * parent, struct rb_node ** rb_link);
void rb_insert_color(struct rb_node *, struct rb_root *);
void rb_erase(struct rb_node *, struct rb_root *);
struct rb_node * rb_first(const struct rb_root *);
struct rb_node * rb_last(const struct rb_root *);
struct rb_node * rb_next(const struct rb_node *);
struct rb_node * rb_prev(const struct rb_node *);
void rb_insert_color_cached(struct rb_node *, struct rb_root_cached *, int);
void rb_erase_cached(struct rb_node *, struct rb_root_cached *);
void rb_replace_node(struct rb_node * victim, struct rb_node * new, struct rb_root * root);
```

- `rb_link_node` — Link node into tree at position indicated by `parent` and `rb_link`
- `rb_insert_color` — Rebalance tree after rb_link_node, completing insertion
- `rb_erase` — Remove node from tree and rebalance
- `rb_first/last/next/prev` — Iterator functions for in-order traversal
- `rb_insert_color_cached` — Insert with cached root, `leftmost` flag set to 1 if node is new leftmost
- `rb_erase_cached` — Erase from cached root, updating leftmost pointer
- `rb_replace_node` — Replace victim node with new node in place, without rebalancing

## Post-order Traversal

```c
#define rbtree_postorder_for_each_entry_safe(pos, n, root, field)
```

Safe post-order traversal for freeing all nodes. Visits children before parent, allowing safe deallocation.

## Example

```c
struct my_entry {
    int key;
    struct rb_node node;
};

struct rb_root root = RB_ROOT;

struct my_entry * entry = xos_mem_malloc(sizeof(struct my_entry));
entry->key = 42;

struct rb_node ** link = &root.rb_node;
struct rb_node * parent = NULL;
while(*link)
{
    parent = *link;
    struct my_entry * e = rb_entry(parent, struct my_entry, node);
    if(entry->key < e->key)
        link = &parent->rb_left;
    else
        link = &parent->rb_right;
}

rb_link_node(&entry->node, parent, link);
rb_insert_color(&entry->node, &root);

struct rb_node * it;
for(it = rb_first(&root); it; it = rb_next(it))
{
    struct my_entry * e = rb_entry(it, struct my_entry, node);
}

struct my_entry * pos, * n;
rbtree_postorder_for_each_entry_safe(pos, n, &root, node)
{
    xos_mem_free(pos);
}
```
