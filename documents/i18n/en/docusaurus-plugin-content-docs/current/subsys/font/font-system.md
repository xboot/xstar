# Font System (font-system)

XSTAR font system is based on TrueType font parsing, supports multi-font family and multi-style installation, providing text measurement and rendering capabilities.

## Architecture

```
font.json → font_install_from_xfs() → Hash table (grouped by style)
                                       ↓
                     font_lookup() ← 3-level fallback lookup strategy
                                       ↓
                 font_text_bound / font_text_render / font_icon_bound / font_icon_render
                                       ↓
                              surface_text / surface_icon
```

## Font Style

| Style | Enum Value | Description |
|-------|------------|-------------|
| FONT_STYLE_REGULAR | 0 | Regular |
| FONT_STYLE_ITALIC | 1 | Italic |
| FONT_STYLE_BOLD | 2 | Bold |
| FONT_STYLE_BOLDITALIC | 3 | Bold Italic |

## Font Configuration

Fonts are configured via `/romdisk/assets/fonts/font.json`, loaded by `do_init_font()` at startup.

```json
{
    "roboto": {
        "regular": "Roboto-Regular.ttf",
        "italic": "Roboto-Italic.ttf",
        "bold": "Roboto-Bold.ttf",
        "bolditalic": "Roboto-BoldItalic.ttf"
    },
    "fallback": {
        "regular": "DroidSansFallback.ttf"
    }
}
```

- The top-level key is the **font family name**
- Keys under each family are style names (`regular`, `italic`, `bold`, `bolditalic`)
- Values are TTF filenames relative to the `/romdisk/assets/fonts/` directory
- A family does not need to include all 4 styles; only the needed ones can be installed

## API

### Font Installation

```c
void font_install_from_xfs(const char * family, enum font_style_t style, struct xfs_context_t * xfs, const char * path);
```

Install a font from the XFS virtual file system. Skips if the family+style already exists.

```c
void font_install_from_buf(const char * family, enum font_style_t style, const void * buf, int len);
```

Install a font from a memory buffer. The caller is responsible for the buffer lifecycle.

```c
void font_uninstall(const char * family, enum font_style_t style);
```

Uninstall the font for the specified family+style and release related resources.

### Text Measurement

```c
int font_text_bound(const char * family, enum font_style_t style, int size, int wrap, const char * str, int * width, int * height);
```

Measure the bounding size of rendered text. `size` is the pixel height, `wrap` is the auto-wrap width (0 means no wrapping), `str` is the UTF-8 string. Results are written to `width` and `height`.

Special character handling:
- `\n` — Newline, pen y advances by one line height
- `\r` — Carriage return, pen x resets to zero
- `\t` — Tab, aligns to multiples of `size * 2`

### Text Rendering

```c
void font_text_render(const char * family, enum font_style_t style, int size, int x, int y, int wrap, const char * str, void (*cb)(void *, int, int, void *, int, int), void * data);
```

Render text per glyph, calling back `cb(data, x0, y0, gray_bitmap, width, height)` for each rendered glyph. `gray_bitmap` is an 8-bit grayscale alpha map.

### Icon Measurement

```c
int font_icon_bound(const char * family, int size, uint32_t code, int * width, int * height);
```

Measure the bounding size of an icon (single Unicode codepoint). Looks up using `FONT_STYLE_REGULAR` style.

### Icon Rendering

```c
void font_icon_render(const char * family, int size, int x, int y, uint32_t code, void (*cb)(void *, int, int, void *, int, int), void * data);
```

Render a single icon glyph. The callback format is the same as text rendering.

### Surface Layer Convenience Interface

```c
void surface_text(struct surface_t * s, struct region_t * clip, int x, int y, int wrap,
    const char * family, enum font_style_t style, int size, struct color_t * c,
    const char * fmt, ...);
```

Render text on a surface, supports printf-style format strings. `c` is the color (defaults to white when NULL).

```c
void surface_icon(struct surface_t * s, struct region_t * clip, int x, int y,
    const char * family, int size, uint32_t code, struct color_t * c);
```

Render an icon on a surface.

## Font Lookup Strategy

`font_lookup()` uses a 3-level fallback strategy to ensure characters always find an available font:

1. **Family name matching** — `family` supports a comma-separated list of family names (e.g., `"roboto,fallback"`), searches the specified style's hash table in order, returns the first font containing the character
2. **Same-style global search** — If no family names match, traverses all fonts under the specified style, returns the first one containing the character
3. **Cross-style global search** — If still not found, traverses fonts of all other styles
4. **Built-in fallback** — If none of the above matches, returns the compile-time built-in fallback font

This strategy ensures that even if the target font lacks a certain character (e.g., CJK characters), it can automatically fall back to a font that contains it.

## Usage Examples

### Installing Custom Fonts

```c
struct xfs_context_t * ctx = xfs_alloc();
font_install_from_xfs("mysans", FONT_STYLE_REGULAR, ctx, "/romdisk/assets/fonts/MySans-Regular.ttf");
font_install_from_xfs("mysans", FONT_STYLE_BOLD, ctx, "/romdisk/assets/fonts/MySans-Bold.ttf");
xfs_free(ctx);
```

### Measuring Text Size

```c
int w, h;
font_text_bound("roboto", FONT_STYLE_REGULAR, 24, 0, "Hello", &w, &h);
```

### Drawing Text on a Surface

```c
struct color_t c = { 255, 0, 0, 255 };
surface_text(s, NULL, 10, 10, 0, "roboto,fallback", FONT_STYLE_REGULAR, 24, &c, "Hello World");
```

### Rendering an Icon

```c
struct color_t c = { 255, 255, 255, 255 };
surface_icon(s, NULL, 10, 10, "material", 32, 0xE001, &c);
```
