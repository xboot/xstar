# LED Strip (ledstrip)

Addressable LED strip.

## Device Type

`DEVICE_TYPE_LEDSTRIP`

## Structure

```c
struct ledstrip_t {
    char * name;
    int (*set_count)(struct ledstrip_t * strip, int count);
    int (*get_count)(struct ledstrip_t * strip);
    int (*set_color)(struct ledstrip_t * strip, int idx, struct color_t * c);
    int (*get_color)(struct ledstrip_t * strip, int idx, struct color_t * c);
    int (*refresh)(struct ledstrip_t * strip);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_ledstrip(name)` | Find LED strip by name |
| `register_ledstrip(strip, drv)` | Register LED strip |
| `unregister_ledstrip(strip)` | Unregister LED strip |
| `ledstrip_set_color(strip, idx, color)` | Set pixel color |
| `ledstrip_refresh(strip)` | Refresh strip display |

## Description

Addressable LED strip interface (e.g., WS2812/NeoPixel). Each pixel's color is set via `struct color_t`, and changes are submitted through `refresh()`.
