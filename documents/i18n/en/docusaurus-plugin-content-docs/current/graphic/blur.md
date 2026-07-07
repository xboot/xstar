# Blur (blur)

Box blur implementation.

## API

```c
void blur(unsigned char * pixel, int width, int height,
          int x, int y, int w, int h, int radius);
```

## Parameters

| Parameter | Description |
|------|------|
| `pixel` | Pixel data pointer |
| `width, height` | Image dimensions |
| `x, y, w, h` | Blur region |
| `radius` | Blur radius |

## Description

Applies a Box blur effect to the specified rectangular region. `radius` controls the blur intensity, with larger values producing stronger blur. Used for visual effects such as frosted glass.
