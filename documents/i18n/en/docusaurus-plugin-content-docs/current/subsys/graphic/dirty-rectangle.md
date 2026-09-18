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
| `dirtylist_add(l, r)` | Add dirty region (plain append, no merging) |
| `dirtylist_optimize(l, n)` | One-shot optimize: rebuild as an exact non-overlapping union, then compress to at most n rects |

## Description

The dirty rectangle list tracks regions that need redrawing. `dirtylist_add()` is a plain O(1) append with no merging; once regions have been accumulated, call `dirtylist_optimize()` to optimize in a single pass: it first rebuilds the list as a pixel-exact, pairwise non-overlapping union via a y-axis band sweep, then repeatedly merges the pair with the least bounding-box penalty until at most n rects remain. When n <= 0, compression is skipped and only the exact union is kept (suitable for present paths with no per-rect transaction overhead).
