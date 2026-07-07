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
| `dirtylist_add(l, r)` | 添加脏区域 |

## 说明

脏矩形列表按区域面积降序排列，用于跟踪需要重绘的区域。合并操作会合并两个列表，清空前先排序以便快速去重。
