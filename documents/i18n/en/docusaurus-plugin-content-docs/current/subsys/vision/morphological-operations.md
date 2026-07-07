# Morphological Operations (morphological-operations)

## API

| Function | Description |
|------|------|
| `vision_dilate(v, times)` | Dilate |
| `vision_erode(v, times)` | Erode |

## Notes

Dilation expands bright regions, erosion shrinks bright regions. `times` controls the number of applications; multiple applications have a cumulative effect. Used for denoising, edge detection, and morphological opening/closing operations.

