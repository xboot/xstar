# Color Conversion (color-conversion)

## API

| Function | Description |
|------|------|
| `vision_gray(v)` | Convert to grayscale |
| `vision_sepia(v)` | Apply sepia tone |
| `vision_invert(v)` | Invert colors |
| `vision_gamma(v, gamma)` | Gamma correction |
| `vision_colormap(v, type)` | Apply color map |

## Color Map Types

| Type | Description |
|------|------|
| `"parula"` | Parula colormap (default) |
| `"jet"` | Jet colormap |
| `"rainbow"` | Rainbow colormap |

## Notes

`vision_gray` converts RGB to grayscale using the luminance formula `0.299R + 0.587G + 0.114B`.
`vision_colormap` operates on RGB images; each channel value is mapped through a lookup table.

