# Color (color)

## Struct

```c
struct color_t {
    uint8_t r;  /* Red */
    uint8_t g;  /* Green */
    uint8_t b;  /* Blue */
    uint8_t a;  /* Alpha */
};
```

Pixel format is 32-bit premultiplied ARGB, native-endian.

## API

| Function | Description |
|------|------|
| `color_init(c, r, g, b, a)` | Initialize color |
| `color_init_string(c, s)` | Parse color from string |
| `color_random(c)` | Generate random color |
| `color_get_premult(c)` | Get premultiplied ARGB value |
| `color_set_premult(c, v)` | Set from premultiplied ARGB |
| `color_luminance(c)` | Calculate luminance (0.3R + 0.59G + 0.11B) |
| `color_mix(c, a, b, weight)` | Mix two colors |
| `color_lighten(c, o, level)` | Lighten |
| `color_darken(c, o, level)` | Darken |
| `color_level(c, o, level)` | Level adjustment [-10, +10] |
| `color_set_hsva(c, h, s, v, a)` | Set from HSVA [0-1] |
| `color_get_hsva(c, h, s, v, a)` | Get HSVA [0-1] |
| `color_premult_blend(dst, len, color, alpha)` | Premultiplied blend |

## String Parsing Format

The following formats are supported:

| Format | Example |
|------|------|
| `#RGB` | `#F00` = Red |
| `#RGBA` | `#F008` = Semi-transparent red |
| `#RRGGBB` | `#FF0000` = Red |
| `#RRGGBBAA` | `#FF000080` = Semi-transparent red |
| `r,g,b,a` | `255,0,0,128` = Semi-transparent red |
| CSS name | `"red"`, `"blue"`, etc. |
