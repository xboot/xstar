# Filter (filter)

Built-in image filters for Surface.

## API

| Function | Description |
|------|------|
| `surface_filter_gray(s)` | Grayscale |
| `surface_filter_sepia(s)` | Sepia tone |
| `surface_filter_invert(s)` | Color invert |
| `surface_filter_coloring(s, c)` | Color tinting |
| `surface_filter_gamma(s, gamma)` | Gamma correction |
| `surface_filter_hue(s, angle)` | Hue rotation (degrees) |
| `surface_filter_saturate(s, saturate)` | Saturation adjustment [-100, 100] |
| `surface_filter_brightness(s, brightness)` | Brightness adjustment [-100, 100] |
| `surface_filter_contrast(s, contrast)` | Contrast adjustment [-100, 100] |
| `surface_filter_opacity(s, alpha)` | Opacity [0, 100] |
| `surface_filter_haldclut(s, clut, type)` | HALD CLUT color lookup table |
| `surface_filter_blur(s, radius)` | Blur |

## Description

`surface_filter_haldclut` supports two interpolation modes:

| Mode | Description |
|------|------|
| `"nearest"` | Nearest neighbor interpolation (fast) |
| `"trilinear"` | Trilinear interpolation (high quality) |

All filters directly modify the Surface pixel data and are irreversible.
