# 2D图形加速 (g2d)

2D 图形加速。

## 设备类型

`DEVICE_TYPE_G2D` (17)

## 结构体

```c
struct g2d_t {
    char * name;
    int (*blit)(struct g2d_t * g2d, struct surface_t * dst, struct region_list_t * dl, struct surface_t * src, struct matrix2d_t * m);
    int (*fill)(struct g2d_t * g2d, struct surface_t * dst, struct region_list_t * dl, struct color_t * c);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_g2d(name)` | 按名称查找 G2D 设备 |
| `register_g2d(g2d, drv)` | 注册 G2D 设备 |
| `unregister_g2d(g2d)` | 注销 G2D 设备 |
| `g2d_blit(g2d, dst, dl, src, m)` | 硬件加速位块传输 |
| `g2d_fill(g2d, dst, dl, c)` | 硬件加速填充 |

## 说明

2D 图形加速硬件接口，将渲染操作从 CPU 卸载到专用图形硬件。
