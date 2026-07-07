# Sorted List (slist)

A sorted key-value list based on a doubly linked list. Each node carries a string key and a user data pointer, supporting sorting by key.

## Data Structure

```c
struct slist_t {
    struct list_head_t list;   /* Doubly linked list node */
    char * key;                /* Sort key (generated via printf formatting) */
    void * priv;               /* User data pointer */
};
```

The `key` and `priv` of the head node are unused; it serves only as a list sentinel. Actual data is stored in child nodes after the head node.

## API

```c
struct slist_t * slist_alloc(void);
```

Allocate an empty list. The head node's `key` is NULL, `list` is initialized as an empty linked list.

```c
void slist_free(struct slist_t * sl);
```

Free the list and all child nodes (including each node's `key` string), but does not free the user data pointed to by `priv`.

> Note: The caller is responsible for managing the memory pointed to by `priv`.

```c
void slist_add(struct slist_t * sl, void * priv, const char * fmt, ...);
```

Add a node. `priv` is a user data pointer, `fmt` is a printf-style format string used to generate the node's `key`. New nodes are appended to the end of the list. If key generation fails, the node is not added.

```c
void slist_sort(struct slist_t * sl);
```

Sort the list by key in lexicographic order (`xos_strcmp`). Uses merge sort, time complexity O(n log n).

```c
void slist_natsort(struct slist_t * sl);
```

Sort the list by key in natural order (`xos_strnatcmp`). Numeric parts are compared by value, e.g. `file1 < file2 < file10` (lexicographic order would give `file1 < file10 < file2`). Uses merge sort, time complexity O(n log n).

```c
void slist_sort_with(struct slist_t * sl, int (*cmp)(void *, struct list_head_t *, struct list_head_t *));
```

Sort the list using a custom comparison function. `cmp` is the comparison callback, with the same semantics as `xos_strcmp` (negative means a < b, 0 means equal, positive means a > b). If `cmp` is NULL, no sorting is performed.

```c
int slist_isempty(struct slist_t * sl);
```

Returns 1 if the list is empty, 0 otherwise.

## Traversal Macros

```c
slist_for_each_entry(entry, sl)
```

Forward traversal of all child nodes in the list, where `entry` is an iteration variable of type `struct slist_t *`.

```c
slist_for_each_entry_reverse(entry, sl)
```

Reverse traversal of all child nodes in the list.

## Usage Examples

### Basic Usage

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
/* Output: apple, banana, cherry */

slist_free(sl);
```

### Formatted Keys

```c
slist_add(sl, data_ptr, "item-%04d", index);
slist_add(sl, data_ptr, "%s/%s", category, name);
```

### Natural Sort

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
/* Output: file1, file2, file10 */

slist_free(sl);
```

### Custom Sort

```c
static int my_compare(void * priv, struct list_head_t * a, struct list_head_t * b)
{
    char * keya = (char *)list_entry(a, struct slist_t, list)->key;
    char * keyb = (char *)list_entry(b, struct slist_t, list)->key;
    return xos_strnatcasecmp(keya, keyb);
}

slist_sort_with(sl, my_compare);
```

### Traversal with Custom Data

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
    /* use data */
}

slist_free(sl);  /* Note: free memory pointed to by priv before slist_free */
```
