# LRU Cache (lru)

Least Recently Used cache backed by hash table + doubly-linked list, evicting least recently used items when capacity is exceeded. Supports eviction callback. Accessed items are automatically promoted to the head of the list.

## Data Structure

```c
struct lru_item_t {
    struct lru_item_t * next;
    struct lru_item_t * prev;
    struct lru_item_t * hnext;
    int nbytes;
    int nkey;
    char data[];
};

struct lru_t {
    unsigned int hashpower;
    size_t max_bytes;
    size_t curr_bytes;
    struct lru_item_t ** table;
    struct lru_item_t * head;
    struct lru_item_t * tail;
    void (*callback)(struct lru_t * l, const char * key, int nkey, void * buf, int nbuf);
};
```

## API

```c
struct lru_t * lru_alloc(size_t maxbytes, unsigned int hashpower, void (*cb)(struct lru_t *, const char *, int, void *, int));
void lru_free(struct lru_t * l);
int lru_get(struct lru_t * l, const char * key, int nkey, void * buf, int nbuf);
int lru_set(struct lru_t * l, const char * key, int nkey, void * buf, int nbuf);
int lru_remove(struct lru_t * l, const char * key, int nkey);
```

- `lru_alloc` — Allocate LRU cache. If `maxbytes` is 0, defaults to 64KB. If `hashpower` is 0, it is automatically derived from `maxbytes` (~64 bytes per bucket, min 6 max 20). `cb` is the eviction callback (can be NULL). Returns NULL on failure
- `lru_free` — Free the cache and all items
- `lru_get` — Look up key of `nkey` bytes, copy up to `nbuf` bytes of value into `buf`. Promotes item to head on hit. Returns number of bytes copied, or 0 if not found
- `lru_set` — Store key-value pair. Evicts from tail if capacity exceeded. Returns `nbuf` on success, 0 on failure
- `lru_remove` — Remove item by key. Returns 1 on success, 0 if not found

## Example

```c
static void on_evict(struct lru_t * l, const char * key, int nkey, void * buf, int nbuf)
{
}

struct lru_t * cache = lru_alloc(4096, 0, on_evict);
if(!cache)
    return;

lru_set(cache, "key1", 4, "value1", 6);

char buf[64];
int n = lru_get(cache, "key1", 4, buf, sizeof(buf));

lru_remove(cache, "key1", 4);
lru_free(cache);
```
