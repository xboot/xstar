# earth

Show a rotating 3D textured earth. Renders the sphere with lookup-table-based trigonometry (`fasin`/`fatan2`) and supports mouse and touch interaction. Falls back to a built-in 1024×512 JPEG earth texture when no texture is given.

## Usage

```
earth [texture] [-f=framebuffer] [-i=input]
```

## Options

| Option | Description |
|------|------|
| `texture` | Optional earth texture file (equirectangular projection); uses the built-in texture when omitted |
| `-f=<fb>` | Target framebuffer device |
| `-i=<input>` | Input device |

## Notes

- The texture is loaded from XFS; a file that resolves but fails to load now reports an error and exits instead of silently falling back to the built-in image
- Auto-rotates horizontally by default, roughly one revolution per 120 seconds
- Supports mouse drag to rotate and wheel to zoom
- Supports single-touch drag to rotate and two-finger pinch to zoom
- Zoom is clamped to 0.2 ~ 5.0×
- Latitude rotation is clamped to ±90°
- Refreshes at roughly 60 FPS, press Ctrl-C to exit

## Examples

```bash
# Use the built-in texture
earth

# Specify framebuffer and input device
earth -f=fb.0 -i=input.0

# Use a custom texture
earth earth_texture.jpg
```
