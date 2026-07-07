# Framebuffer (framebuffer)

Framebuffer display.

## Device Type

`DEVICE_TYPE_FRAMEBUFFER`

## Struct

```c
struct framebuffer_t {
    char * name;
    int width, height, pwidth, pheight;
    int (*setbl)(struct framebuffer_t * fb, int brightness);
    int (*getbl)(struct framebuffer_t * fb);
    struct surface_t * (*create)(struct framebuffer_t * fb);
    void (*destroy)(struct framebuffer_t * fb, struct surface_t * s);
    void (*present)(struct framebuffer_t * fb, struct surface_t * s, struct region_list_t * l);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_framebuffer(name)` | Find framebuffer by name |
| `register_framebuffer(fb, drv)` | Register framebuffer |
| `unregister_framebuffer(fb)` | Unregister framebuffer |
| `framebuffer_get/set_backlight(fb, bl)` | Get/set backlight |

## Description

Video framebuffer display interface. Manages pixel buffer, supports surface creation and dirty-rectangle based presentation.
