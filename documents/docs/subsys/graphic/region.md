# 区域 (region)

矩形区域，用于裁剪、碰撞检测和脏区域跟踪。

## 结构体

```c
struct region_t {
    int x, y;
    int w, h;
};
```

## API

| 函数 | 说明 |
|------|------|
| `region_init(r, x, y, w, h)` | 初始化区域 |
| `region_clone(r, o)` | 克隆区域 |
| `region_isempty(r)` | 判断是否为空 |
| `region_hit(r, x, y)` | 点包含测试 |
| `region_contains(r, o)` | 区域包含测试 |
| `region_overlap(r, o)` | 重叠测试 |
| `region_expand(r, o, n)` | 扩展区域 |
| `region_intersect(r, a, b)` | 求交集，返回是否非空 |
| `region_union(r, a, b)` | 求并集 |
