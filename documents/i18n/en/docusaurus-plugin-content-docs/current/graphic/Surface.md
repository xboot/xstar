# Surface (surface)

The core object of the graphics system, representing a 32-bit premultiplied ARGB pixel buffer.

## Struct

```c
struct surface_t {
    int width;
    int height;
    int stride;
    int pixlen;
    void * pixels;
    struct render_t * r;
    void * rctx;
    void * g2d;
    void * priv;
};
```

## Creation & Loading

| Function | Description |
|------|------|
| `surface_alloc(width, height)` | Create a blank Surface |
| `surface_alloc_from_xfs(ctx, filename)` | Load image from XFS |
| `surface_alloc_from_buf(buf, len)` | Load from memory buffer |
| `surface_alloc_qrcode(pixsz, fmt, ...)` | Generate QR code Surface |
| `surface_free(s)` | Free Surface |
| `surface_clone(s, x, y, w, h)` | Clone sub-region |
| `surface_extend(s, w, h, type)` | Extend dimensions |

## Basic Operations

| Function | Description |
|------|------|
| `surface_clear(s, c, x, y, w, h)` | Clear region |
| `surface_set_pixel(s, x, y, c)` | Set pixel |
| `surface_get_pixel(s, x, y, c)` | Get pixel |
| `surface_blit(s, clip, m, o)` | Bit block transfer (with clipping and matrix transform) |
| `surface_fill(s, clip, m, w, h, c)` | Fill rectangle |
| `surface_text(s, clip, x, y, wrap, family, style, size, c, fmt, ...)` | Render text |
| `surface_icon(s, clip, x, y, family, size, code, c)` | Render icon glyph |

## Property Access

| Function | Description |
|------|------|
| `surface_get_width(s)` | Get width |
| `surface_get_height(s)` | Get height |
| `surface_get_stride(s)` | Get stride |
| `surface_get_pixels(s)` | Get pixel pointer |
| `surface_get_pixlen(s)` | Get pixel data length |
