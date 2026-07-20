# Surface (surface)

图形系统的核心对象，表示一个 32 位预乘 ARGB 像素缓冲。

## 结构体

```c
struct surface_t {
    int width;
    int height;
    int stride;
    int pixlen;
    void * pixels;
    struct g2d_t * g2d;
    struct cg_render_t * cg;
    void * priv;
};
```

## 创建与加载

| 函数 | 说明 |
|------|------|
| `surface_alloc(width, height)` | 创建空白 Surface |
| `surface_alloc_from_xfs(ctx, filename)` | 从 XFS 加载图像 |
| `surface_alloc_from_buf(buf, len)` | 从内存缓冲加载 |
| `surface_alloc_qrcode(pixsz, fmt, ...)` | 生成二维码 Surface |
| `surface_free(s)` | 释放 Surface |
| `surface_clone(s, x, y, w, h)` | 克隆子区域 |
| `surface_extend(s, w, h, type)` | 扩展尺寸 |

## 基本操作

| 函数 | 说明 |
|------|------|
| `surface_clear(s, c, x, y, w, h)` | 清除区域 |
| `surface_set_pixel(s, x, y, c)` | 设置像素 |
| `surface_get_pixel(s, x, y, c)` | 获取像素 |
| `surface_blit(s, clip, m, o)` | 位块传输（带裁剪和矩阵变换） |
| `surface_fill(s, clip, m, w, h, c)` | 填充矩形 |
| `surface_text(s, clip, x, y, wrap, family, style, size, c, fmt, ...)` | 渲染文本 |
| `surface_icon(s, clip, x, y, family, size, code, c)` | 渲染图标符号 |

## 获取属性

| 函数 | 说明 |
|------|------|
| `surface_get_width(s)` | 获取宽度 |
| `surface_get_height(s)` | 获取高度 |
| `surface_get_stride(s)` | 获取跨度 |
| `surface_get_pixels(s)` | 获取像素指针 |
| `surface_get_pixlen(s)` | 获取像素数据长度 |

## CG 集成

| 函数 | 说明 |
|------|------|
| `surface_get_cg_surface(s)` | 获取关联的 `cg_surface_t`（惰性创建） |
| `surface_get_cg_ctx(s)` | 获取关联的 `cg_ctx_t`（惰性创建） |

通过 cg 上下文可进行矢量绘图（路径、填充、描边、渐变等），详见[矢量绘图](../cg/cg)文档。`surface_blit()` 和 `surface_fill()` 在 G2D 硬件加速不可用时自动回退到 cg 渲染。
