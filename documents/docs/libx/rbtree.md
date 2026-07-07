# 红黑树 (rbtree)

红黑树，自平衡二叉搜索树，遵循 Linux 内核风格。支持普通根和缓存根（O(1) 最小值查找），以及后序遍历（安全删除）。

## 宏

```c
rb_entry(ptr, type, member)
RB_ROOT
RB_ROOT_CACHED
rb_first_cached(root)
rbtree_postorder_for_each_entry_safe(pos, n, root, field)
```

## API

```c
void rb_link_node(struct rb_node * node, struct rb_node * parent, struct rb_node ** rb_link);
void rb_insert_color(struct rb_node *, struct rb_root *);
void rb_erase(struct rb_node *, struct rb_root *);
struct rb_node * rb_first(struct rb_root *);
struct rb_node * rb_last(struct rb_root *);
struct rb_node * rb_next(struct rb_node *);
struct rb_node * rb_prev(struct rb_node *);
void rb_insert_color_cached(struct rb_node *, struct rb_root_cached *, int leftmost);
void rb_erase_cached(struct rb_node *, struct rb_root_cached *);
void rb_replace_node(struct rb_node * victim, struct rb_node * new, struct rb_root * root);
void rb_replace_node_cached(struct rb_node * victim, struct rb_node * new, struct rb_root_cached * root);
```

- `rb_link_node` — 链接节点（不重平衡）
- `rb_insert_color` — 插入后重平衡
- `rb_erase` — 删除节点并重平衡
- `rb_first` / `rb_last` / `rb_next` / `rb_prev` — 遍历
- `rb_insert_color_cached` / `rb_erase_cached` — 缓存根版本
- `rb_replace_node` / `rb_replace_node_cached` — 替换节点（不重平衡）

## 使用示例

```c
struct my_node {
	struct rb_node rb;
	int key;
};

struct rb_root root = RB_ROOT;

struct my_node * new_node = malloc(sizeof(*new_node));
new_node->key = 42;

struct rb_node ** link = &root.rb_node;
struct rb_node * parent = NULL;
while (*link)
{
	parent = *link;
	struct my_node * entry = rb_entry(parent, struct my_node, rb);
	if (new_node->key < entry->key)
		link = &parent->rb_left;
	else
		link = &parent->rb_right;
}

rb_link_node(&new_node->rb, parent, link);
rb_insert_color(&new_node->rb, &root);

struct rb_node * node;
rbtree_postorder_for_each_entry_safe(node, n, &root, rb)
	free(rb_entry(node, struct my_node, rb));
```
