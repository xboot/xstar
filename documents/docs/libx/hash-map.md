# 哈希表 (hmap)

基于哈希的键值映射表，支持自动扩缩容、按键排序、线程安全和回调通知。

## 数据结构

```c
struct hmap_entry_t {
    struct hlist_node_t node;   /* 哈希桶链表节点 */
    struct list_head_t head;    /* 全局有序链表节点 */
    char * key;                 /* 键（内部拷贝） */
    void * value;               /* 值（用户数据指针） */
};

struct hmap_t {
    struct hlist_head_t * hash;  /* 哈希桶数组 */
    struct list_head_t list;     /* 全局链表（插入顺序） */
    unsigned int size;           /* 哈希桶数量 */
    unsigned int n;              /* 当前条目数 */
    struct mutex_t lock;         /* 互斥锁 */
    void (*callback)(struct hmap_t *, struct hmap_entry_t *);  /* 条目释放回调 */
};
```

## API

```c
struct hmap_t * hmap_alloc(int size, void (*cb)(struct hmap_t *, struct hmap_entry_t *));
```

分配哈希表。`size` 为初始桶数量（最小 16，自动取整到 2 的幂），`cb` 为条目释放时的回调（可为 NULL）。回调在 `hmap_clear` 和 `hmap_free` 删除条目时触发，用于释放 `value` 指向的用户资源。

```c
void hmap_free(struct hmap_t * m);
```

释放哈希表，先清空所有条目（触发回调），再释放内部资源。

```c
void hmap_clear(struct hmap_t * m);
```

清空所有条目。每删除一个条目前触发回调，可用于释放 `value` 资源。

```c
void hmap_add(struct hmap_t * m, const char * key, void * value);
```

添加键值对。若 key 已存在则更新 value。内部自动扩容（元素数超过桶数一半时桶数翻倍）。

```c
void hmap_remove(struct hmap_t * m, const char * key);
```

删除指定 key 的条目。key 不存在时无操作。自动缩容（元素数低于桶数一半且桶数大于 16 时桶数减半）。

```c
void * hmap_search(struct hmap_t * m, const char * key);
```

查找 key 对应的 value，未找到返回 NULL。

```c
void hmap_sort(struct hmap_t * m);
```

按 key 的字典序排序全局链表，影响 `hmap_for_each_entry` 的遍历顺序。

```c
void hmap_natsort(struct hmap_t * m);
```

按 key 的自然序（natural sort）排序全局链表。与 `hmap_sort` 的字典序不同，自然序将数字按数值大小比较，例如 `file2` 排在 `file10` 之前（字典序则 `file10` 排在 `file2` 之前）。

```c
void hmap_sort_with(struct hmap_t * m, int (*cmp)(void *, struct list_head_t *, struct list_head_t *));
```

使用自定义比较函数排序全局链表。`cmp` 为比较回调，参数 `a`、`b` 为 `struct list_head_t *`，可通过 `list_entry` 获取对应的 `struct hmap_entry_t`。`cmp` 返回值语义与 `lsort` 一致：负数表示 a 在前，正数表示 b 在前，0 表示相等。`m` 或 `cmp` 为 NULL 时无操作。

## 遍历宏

```c
hmap_for_each_entry(entry, m)
```

按链表顺序正序遍历，`entry` 为 `struct hmap_entry_t *` 类型。

```c
hmap_for_each_entry_reverse(entry, m)
```

逆序遍历。

## 自动扩缩容

| 条件 | 动作 |
|------|------|
| n > size / 2 | 桶数翻倍 |
| n < size / 2 且 size > 16 | 桶数减半 |

扩缩容时重新计算所有条目的哈希位置，开销较高但保证查找效率。

## 使用示例

### 基本使用

```c
struct hmap_t * m = hmap_alloc(16, NULL);
hmap_add(m, "name", "alice");
hmap_add(m, "age", (void *)30);

char * name = (char *)hmap_search(m, "name");
hmap_remove(m, "age");
hmap_free(m);
```

### 带回调的资源管理

```c
static void my_callback(struct hmap_t * m, struct hmap_entry_t * e)
{
    xos_mem_free(e->value);
}

struct hmap_t * m = hmap_alloc(16, my_callback);

char * data = xos_mem_malloc(100);
hmap_add(m, "key1", data);

hmap_free(m);  /* 自动调用 my_callback 释放 data */
```

### 排序遍历

```c
hmap_add(m, "banana", (void *)2);
hmap_add(m, "apple", (void *)1);
hmap_add(m, "cherry", (void *)3);

hmap_sort(m);

struct hmap_entry_t * entry;
hmap_for_each_entry(entry, m)
{
    printf("%s = %d\n", entry->key, (int)(uintptr_t)entry->value);
}
/* 输出按字典序: apple, banana, cherry */
```

### 自然序排序

```c
hmap_add(m, "file1", (void *)1);
hmap_add(m, "file10", (void *)10);
hmap_add(m, "file2", (void *)2);

hmap_natsort(m);

struct hmap_entry_t * entry;
hmap_for_each_entry(entry, m)
{
    printf("%s = %d\n", entry->key, (int)(uintptr_t)entry->value);
}
/* 输出按自然序: file1, file2, file10 */
```

### 自定义排序

```c
static int my_compare(void * priv, struct list_head_t * a, struct list_head_t * b)
{
    char * keya = (char *)list_entry(a, struct hmap_entry_t, head)->key;
    char * keyb = (char *)list_entry(b, struct hmap_entry_t, head)->key;
    return xos_strcmp(keya, keyb);
}

hmap_sort_with(m, my_compare);
```
