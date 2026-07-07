# 2D Graphics Accelerator (g2d)

2D graphics acceleration.

## Device Type

`DEVICE_TYPE_G2D` (17)

## Struct

```c
struct g2d_t {
    char * name;
    int (*blit)(struct g2d_t * g2d, struct surface_t * dst, struct region_list_t * dl, struct surface_t * src, struct matrix2d_t * m);
    int (*fill)(struct g2d_t * g2d, struct surface_t * dst, struct region_list_t * dl, struct color_t * c);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_g2d(name)` | Find G2D device by name |
| `register_g2d(g2d, drv)` | Register G2D device |
| `unregister_g2d(g2d)` | Unregister G2D device |
| `g2d_blit(g2d, dst, dl, src, m)` | Hardware accelerated blit |
| `g2d_fill(g2d, dst, dl, c)` | Hardware accelerated fill |

## Description

2D graphics acceleration hardware interface, offloading rendering operations from CPU to dedicated graphics hardware.
