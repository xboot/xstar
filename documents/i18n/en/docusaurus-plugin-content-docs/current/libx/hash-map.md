# Hash Map (hmap)

Hash-based key-value mapping table, supporting automatic resize, key sorting, thread safety, and callback notifications.

## Data Structure

```c
struct hmap_entry_t {
    struct hlist_node_t node;   /* Hash bucket linked list node */
    struct list_head_t head;    /* Global ordered linked list node */
    char * key;                 /* Key (internally copied) */
    void * value;               /* Value (user data pointer) */
};

struct hmap_t {
    struct hlist_head_t * hash;  /* Hash bucket array */
    struct list_head_t list;     /* Global linked list (insertion order) */
    unsigned int size;           /* Number of hash buckets */
    unsigned int n;              /* Current entry count */
    struct mutex_t lock;         /* Mutex lock */
    void (*callback)(struct hmap_t *, struct hmap_entry_t *);  /* Entry release callback */
};
```

## API

```c
struct hmap_t * hmap_alloc(int size, void (*cb)(struct hmap_t *, struct hmap_entry_t *));
```

Allocate a hash table. `size` is the initial number of buckets (minimum 16, automatically rounded to a power of 2), `cb` is the callback when an entry is released (can be NULL). The callback is triggered when entries are deleted by `hmap_clear` and `hmap_free`, used to free user resources pointed to by `value`.

```c
void hmap_free(struct hmap_t * m);
```

Free the hash table, first clearing all entries (triggering callbacks), then freeing internal resources.

```c
void hmap_clear(struct hmap_t * m);
```

Clear all entries. The callback is triggered before each entry is deleted, can be used to release `value` resources.

```c
void hmap_add(struct hmap_t * m, const char * key, void * value);
```

Add a key-value pair. Updates the value if the key already exists. Automatically expands (bucket count doubles when the number of elements exceeds half the bucket count).

```c
void hmap_remove(struct hmap_t * m, const char * key);
```

Remove the entry with the specified key. No operation if the key does not exist. Automatically shrinks (bucket count halves when the number of elements is below half the bucket count and bucket count is greater than 16).

```c
void * hmap_search(struct hmap_t * m, const char * key);
```

Search for the value corresponding to the key, returns NULL if not found.

```c
void hmap_sort(struct hmap_t * m);
```

Sort the global linked list by key lexicographic order, affecting the traversal order of `hmap_for_each_entry`.

```c
void hmap_natsort(struct hmap_t * m);
```

Sort the global linked list by key in natural order. Unlike `hmap_sort` which uses lexicographic order, natural sort compares numbers by their numeric value, e.g. `file2` comes before `file10` (lexicographic order would place `file10` before `file2`).

```c
void hmap_sort_with(struct hmap_t * m, int (*cmp)(void *, struct list_head_t *, struct list_head_t *));
```

Sort the global linked list using a custom comparison function. `cmp` is the comparison callback, where `a` and `b` are `struct list_head_t *`, and the corresponding `struct hmap_entry_t` can be obtained via `list_entry`. The return value semantics of `cmp` are consistent with `lsort`: negative means a comes first, positive means b comes first, zero means equal. No operation if `m` or `cmp` is NULL.

## Traversal Macros

```c
hmap_for_each_entry(entry, m)
```

Forward traversal in list order, `entry` is of type `struct hmap_entry_t *`.

```c
hmap_for_each_entry_reverse(entry, m)
```

Reverse traversal.

## Automatic Resize

| Condition | Action |
|------|------|
| n > size / 2 | Bucket count doubles |
| n < size / 2 and size > 16 | Bucket count halves |

During resize, all entries are rehashed to new positions, which is costly but ensures lookup efficiency.

## Usage Examples

### Basic Usage

```c
struct hmap_t * m = hmap_alloc(16, NULL);
hmap_add(m, "name", "alice");
hmap_add(m, "age", (void *)30);

char * name = (char *)hmap_search(m, "name");
hmap_remove(m, "age");
hmap_free(m);
```

### Resource Management with Callback

```c
static void my_callback(struct hmap_t * m, struct hmap_entry_t * e)
{
    xos_mem_free(e->value);
}

struct hmap_t * m = hmap_alloc(16, my_callback);

char * data = xos_mem_malloc(100);
hmap_add(m, "key1", data);

hmap_free(m);  /* automatically calls my_callback to free data */
```

### Sorted Traversal

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
/* Output in lexicographic order: apple, banana, cherry */
```

### Natural Sort

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
/* Output in natural order: file1, file2, file10 */
```

### Custom Sort

```c
static int my_compare(void * priv, struct list_head_t * a, struct list_head_t * b)
{
    char * keya = (char *)list_entry(a, struct hmap_entry_t, head)->key;
    char * keyb = (char *)list_entry(b, struct hmap_entry_t, head)->key;
    return xos_strcmp(keya, keyb);
}

hmap_sort_with(m, my_compare);
```
