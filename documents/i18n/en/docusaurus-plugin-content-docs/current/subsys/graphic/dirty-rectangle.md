# Dirty Rectangle (dirty-rectangle)

Track regions that need updating, optimizing incremental rendering.

## Struct

```c
struct dirtylist_t {
    struct region_t * items;
    unsigned int nitems;
    unsigned int size;
    void * priv;
};
```

## API

| Function | Description |
|------|------|
| `dirtylist_alloc(size)` | Allocate a dirty rectangle list; capacity is at least 16 and rounded up to a power of two |
| `dirtylist_free(l)` | Free |
| `dirtylist_clone(l, o)` | Clone |
| `dirtylist_merge(l, o)` | Merge |
| `dirtylist_clear(l)` | Clear |
| `dirtylist_add(l, r)` | Add dirty region (plain append, no merging) |
| `dirtylist_optimize(l, n)` | One-shot optimize: rebuild as an exact non-overlapping union, then compress to at most n rects |

## Description

The dirty rectangle list tracks regions that need redrawing. `dirtylist_add()` is a plain O(1) append with no merging. The list and its workspace grow with a minimum capacity of 16 and power-of-two capacities, allowing storage to be reused while avoiding frequent reallocations.

Once regions have been accumulated, call `dirtylist_optimize()` to optimize them in one operation. It first rebuilds the list as a pixel-exact, pairwise non-overlapping union via a y-axis band sweep, then repeatedly merges the pair with the least bounding-box penalty until at most `n` rectangles remain. The penalty accounts for the intersection of the two rectangles and therefore represents the pixel area added by that merge. When `n <= 0`, compression is skipped and only the exact union is kept, which is suitable for present paths with no per-rectangle transaction overhead. To bound the subsequent merge cost in extremely fragmented cases, an exact union containing more than 32 rectangles first falls back to one bounding rectangle covering all damage.
