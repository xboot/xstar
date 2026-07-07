# Thresholding (thresholding)

## API

| Function | Description |
|------|------|
| `vision_threshold(v, threshold, type)` | Apply threshold |
| `vision_inrange(v, lower, upper)` | Range detection, returns a binary image |

## Threshold Types

| Type | Description |
|------|------|
| `"binary"` | pixel > threshold → 255, otherwise 0 |
| `"binary-invert"` | pixel > threshold → 0, otherwise 255 |
| `"tozero"` | pixel ≤ threshold → 0, otherwise unchanged |
| `"tozero-invert"` | pixel > threshold → 0, otherwise unchanged |
| `"truncate"` | pixel > threshold → threshold, otherwise unchanged |

## Notes

When `threshold < 0` or `> 255`, the **Otsu method** is automatically used to compute the optimal threshold.

`vision_inrange` returns a newly allocated grayscale image where pixels in the range `[lower, upper]` are set to 255, otherwise 0. Commonly used for color detection and image segmentation.

