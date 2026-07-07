# Drawing (drawing)

Draw basic shapes and text on Vision images.

## API

| Function | Description |
|------|------|
| `vision_rectangle(v, x, y, w, h, thickness, c)` | Draw a rectangle |
| `vision_text(v, x, y, wrap, family, style, size, c, fmt, ...)` | Render text |
| `vision_icon(v, x, y, family, size, code, c)` | Render an icon glyph |

## Notes

| Function | Description |
|------|------|
| `vision_rectangle` | Draw a rectangle outline; `thickness` is the line width |
| `vision_text` | Render text at a given position, supports line wrapping, font style, and size |
| `vision_icon` | Render an icon glyph from a font file (e.g., FontAwesome) |

