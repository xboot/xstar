# 排序列表 (slist)

基于双向链表的键值排序列表，每个节点携带字符串键和用户数据指针，支持按键排序。

## 数据结构

```c
struct slist_t {
    struct list_head_t list;   /* 双向链表节点 */
    char * key;                /* 排序键（printf 格式化生成） */
    void * priv;               /* 用户数据指针 */
};
```

头节点的 `key` 和 `priv` 不使用，仅作为链表哨兵。实际数据存储在头节点之后的子节点中。

## API

```c
struct slist_t * slist_alloc(void);
```

分配空列表。头节点的 `key` 为 NULL，`list` 初始化为空链表。

```c
void slist_free(struct slist_t * sl);
```

释放列表及所有子节点（包括各节点的 `key` 字符串），但不释放 `priv` 指向的用户数据。

> 注意：调用者需自行管理 `priv` 指向的内存。

```c
void slist_add(struct slist_t * sl, void * priv, const char * fmt, ...);
```

添加节点。`priv` 为用户数据指针，`fmt` 为 printf 风格的格式化字符串，生成节点的 `key`。新节点追加到链表尾部。`key` 生成失败时节点不会被添加。

```c
void slist_sort(struct slist_t * sl);
```

按键的字典序（`xos_strcmp`）对列表排序。使用归并排序，时间复杂度 O(n log n)。

```c
void slist_natsort(struct slist_t * sl);
```

按键的自然排序（`xos_strnatcmp`，natural sort）对列表排序。数字部分按数值比较，例如 `file1 < file2 < file10`（字典序则为 `file1 < file10 < file2`）。使用归并排序，时间复杂度 O(n log n)。

```c
void slist_sort_with(struct slist_t * sl, int (*cmp)(void *, struct list_head_t *, struct list_head_t *));
```

使用自定义比较函数对列表排序。`cmp` 为比较回调，语义与 `xos_strcmp` 一致（返回负数表示 a < b，0 表示相等，正数表示 a > b）。`cmp` 为 NULL 时不执行排序。

```c
int slist_isempty(struct slist_t * sl);
```

列表为空返回 1，非空返回 0。

## 遍历宏

```c
slist_for_each_entry(entry, sl)
```

正序遍历列表中所有子节点，`entry` 为 `struct slist_t *` 类型的迭代变量。

```c
slist_for_each_entry_reverse(entry, sl)
```

逆序遍历列表中所有子节点。

## 使用示例

### 基本使用

```c
struct slist_t * sl = slist_alloc();
slist_add(sl, (void *)10, "banana");
slist_add(sl, (void *)20, "apple");
slist_add(sl, (void *)30, "cherry");

slist_sort(sl);

struct slist_t * entry;
slist_for_each_entry(entry, sl)
{
    printf("key=%s, priv=%d\n", entry->key, (int)(uintptr_t)entry->priv);
}
/* 输出: apple, banana, cherry */

slist_free(sl);
```

### 格式化键

```c
slist_add(sl, data_ptr, "item-%04d", index);
slist_add(sl, data_ptr, "%s/%s", category, name);
```

### 自然排序

```c
struct slist_t * sl = slist_alloc();
slist_add(sl, (void *)1, "file1");
slist_add(sl, (void *)2, "file10");
slist_add(sl, (void *)3, "file2");

slist_natsort(sl);

struct slist_t * entry;
slist_for_each_entry(entry, sl)
{
    printf("key=%s\n", entry->key);
}
/* 输出: file1, file2, file10 */

slist_free(sl);
```

### 自定义排序

```c
static int my_compare(void * priv, struct list_head_t * a, struct list_head_t * b)
{
    char * keya = (char *)list_entry(a, struct slist_t, list)->key;
    char * keyb = (char *)list_entry(b, struct slist_t, list)->key;
    return xos_strnatcasecmp(keya, keyb);
}

slist_sort_with(sl, my_compare);
```

### 带自定义数据的遍历

```c
struct my_data_t { int x; int y; };

struct slist_t * sl = slist_alloc();
struct my_data_t * d = xos_mem_malloc(sizeof(struct my_data_t));
d->x = 1; d->y = 2;
slist_add(sl, d, "key1");

struct slist_t * entry;
slist_for_each_entry(entry, sl)
{
    struct my_data_t * data = (struct my_data_t *)entry->priv;
    /* 使用 data */
}

slist_free(sl);  /* 注意：需在 free 前释放 priv 指向的内存 */
```
