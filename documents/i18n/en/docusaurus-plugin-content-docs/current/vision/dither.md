# Dither (dither)

Error diffusion dithering for reducing images to fewer colors.

## API

| Function | Description |
|------|------|
| `vision_dither(v, type)` | Error diffusion dithering |
| `vision_dither_palette(v, palette, n)` | Custom palette Floyd-Steinberg dithering |

## Dithering Algorithms

| Type | Description |
|------|------|
| `"floyd-steinberg"` | Floyd-Steinberg error diffusion |
| `"jarvis-judice-ninke"` | Jarvis-Judice-Ninke error diffusion |
| `"stucki"` | Stucki error diffusion |
| `"atkinson"` | Atkinson error diffusion |

## Notes

`vision_dither` applies only to grayscale images; errors are diffused to neighboring pixels.

`vision_dither_palette` uses Floyd-Steinberg with a custom palette:
- Grayscale images default to a 2-color (black/white) palette
- RGB images default to an 8 standard color palette

