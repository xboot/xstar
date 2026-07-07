# iplay

Display images on screen.

## Usage

```
iplay [dir|file] [-t=millisecond] [-m=none|contain|cover|fill] [-c=color] [-d=framebuffer]
```

## Options

| Option | Description |
|--------|-------------|
| `-t=<ms>` | Display time per image (default 0 — no delay) |
| `-m=<mode>` | Scale mode: `none` (original size), `contain` (fit), `cover` (fill and crop), `fill` (stretch) |
| `-c=<color>` | Background fill color (hex, e.g. `#000000`) |
| `-d=<fb>` | Target framebuffer device name |

## Notes

- Displays an image file; when a directory is specified, displays all files in sorted order
- Supports common image formats
- Skips files starting with `.`

## Examples

```bash
# Display a single image
iplay image.png

# Play a directory as slideshow
iplay /images -t=2000 -m=contain
```
