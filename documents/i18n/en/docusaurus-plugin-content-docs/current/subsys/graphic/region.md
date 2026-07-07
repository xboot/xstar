# Region (region)

Rectangular region for clipping, collision detection, and dirty rectangle tracking.

## Struct

```c
struct region_t {
    int x, y;
    int w, h;
};
```

## API

| Function | Description |
|------|------|
| `region_init(r, x, y, w, h)` | Initialize region |
| `region_clone(r, o)` | Clone region |
| `region_isempty(r)` | Check if empty |
| `region_hit(r, x, y)` | Point containment test |
| `region_contains(r, o)` | Region containment test |
| `region_overlap(r, o)` | Overlap test |
| `region_expand(r, o, n)` | Expand region |
| `region_intersect(r, a, b)` | Compute intersection, returns whether non-empty |
| `region_union(r, a, b)` | Compute union |
