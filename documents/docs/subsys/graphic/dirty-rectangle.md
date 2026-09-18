# 脏矩形 (dirty-rectangle)

跟踪需要更新的区域，优化增量渲染。

## 结构体

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

| 函数 | 说明 |
|------|------|
| `dirtylist_alloc(size)` | 分配脏矩形列表 |
| `dirtylist_free(l)` | 释放 |
| `dirtylist_clone(l, o)` | 克隆 |
| `dirtylist_merge(l, o)` | 合并 |
| `dirtylist_clear(l)` | 清空 |
| `dirtylist_add(l, r)` | 添加脏区域（纯追加，不做合并） |
| `dirtylist_optimize(l, n)` | 一次性优化：重建为精确不重叠并集，再压缩至最多 n 个矩形 |

## 说明

脏矩形列表用于跟踪需要重绘的区域。`dirtylist_add()` 仅做 O(1) 追加，不做任何合并；区域累积完成后调用 `dirtylist_optimize()` 一次性优化：先通过 y 轴分段扫描将列表重建为像素级精确、两两不重叠的并集，再反复合并包围盒代价最小的一对矩形，直到矩形数不超过 n。n <= 0 时跳过压缩，仅保留精确并集结果（适用于无逐矩形事务开销的呈现路径）。
