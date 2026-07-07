# 变换矩阵 (transform-matrix)

2D 仿射变换矩阵，表示为 3x3 矩阵：`[a b 0; c d 0; tx ty 1]`。

## 结构体

```c
struct matrix2d_t {
    float a; float b;
    float c; float d;
    float tx; float ty;
};
```

## API

| 函数 | 说明 |
|------|------|
| `matrix2d_init(m, a, b, c, d, tx, ty)` | 初始化矩阵 |
| `matrix2d_init_identity(m)` | 单位矩阵 |
| `matrix2d_init_translate(m, tx, ty)` | 平移 |
| `matrix2d_init_scale(m, sx, sy)` | 缩放 |
| `matrix2d_init_rotate(m, r)` | 旋转 |
| `matrix2d_multiply(m, m1, m2)` | 矩阵相乘 |
| `matrix2d_invert(m)` | 求逆 |
| `matrix2d_translate(m, tx, ty)` | 追加平移 |
| `matrix2d_scale(m, sx, sy)` | 追加缩放 |
| `matrix2d_rotate(m, r)` | 追加旋转 |
| `matrix2d_transform_point(m, x, y)` | 变换点 |
| `matrix2d_transform_distance(m, dx, dy)` | 变换距离 |
| `matrix2d_transform_bounds(m, x1, y1, x2, y2)` | 变换边界 |
| `matrix2d_transform_region(m, w, h, region)` | 变换区域 |
