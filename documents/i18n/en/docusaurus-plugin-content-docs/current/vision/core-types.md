# Core Types (core-types)

## Pixel Format

```c
enum vision_type_t {
    VISION_TYPE_GRAY = 0x0110,  /* Grayscale - unsigned char (0~255) */
    VISION_TYPE_RGB  = 0x0311,  /* RGB888 - unsigned char (0~255) */
};
```

Type encoding rule: upper 8 bits are the number of channels, lower 4 bits are the bytes per channel.

## Image Structure

```c
struct vision_t {
    enum vision_type_t type;   /* Pixel format */
    int width;                 /* Width */
    int height;                /* Height */
    int npixel;                /* Total number of pixels */
    void * datas;              /* Plane data */
    size_t ndata;              /* Total data size */
};
```

Grayscale images have 1 plane, RGB images have 3 planes.

## Inline Functions

| Function | Description |
|------|------|
| `vision_type_get_bytes(type)` | Get bytes per channel |
| `vision_type_get_channels(type)` | Get number of channels |
| `vision_get_type(v)` | Get pixel format |
| `vision_get_width(v)` | Get width |
| `vision_get_height(v)` | Get height |
| `vision_get_npixel(v)` | Get pixel count |
| `vision_get_datas(v)` | Get data pointer |
| `vision_get_ndata(v)` | Get data size |

## Lifecycle

| Function | Description |
|------|------|
| `vision_alloc(type, width, height)` | Allocate an image |
| `vision_free(v)` | Free an image |
| `vision_clone(v, x, y, w, h)` | Clone a sub-region as a new image |
| `vision_convert(v, type)` | Convert pixel format |
| `vision_clear(v)` | Clear all pixels to zero |

## Interoperating with Surface

| Function | Description |
|------|------|
| `vision_apply_surface(v, s)` | Copy Surface pixels to Vision |
| `surface_apply_vision(s, v)` | Copy Vision pixels to Surface |
