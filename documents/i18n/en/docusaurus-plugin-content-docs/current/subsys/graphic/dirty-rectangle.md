# Dirty Rectangle (dirty-rectangle)

Track regions that need updating, optimizing incremental rendering.

## Struct

```c
struct dirtylist_item_t {
    struct region_t region;
    int area;
};

struct dirtylist_t {
    struct dirtylist_item_t * items;
    unsigned int size;
    unsigned int count;
};
```

## API

| Function | Description |
|------|------|
| `dirtylist_alloc(size)` | Allocate dirty rectangle list |
| `dirtylist_free(l)` | Free |
| `dirtylist_clone(l, o)` | Clone |
| `dirtylist_merge(l, o)` | Merge |
| `dirtylist_clear(l)` | Clear |
| `dirtylist_add(l, r)` | Add dirty region |

## Description

The dirty rectangle list is sorted in descending order by area, used to track regions that need redrawing. The merge operation combines two lists, and sorting is performed before clearing for fast deduplication.
