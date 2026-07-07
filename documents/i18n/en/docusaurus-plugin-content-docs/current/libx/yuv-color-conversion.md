# YUV Color Conversion (yuv)

YUV to RGB color space conversion module, supporting both BT.601 and BT.709 standards, using integer arithmetic to avoid floating-point dependencies.

## Color Spaces

| Standard | Typical Use |
|----------|-------------|
| BT.601 | Standard definition (SD) video, e.g., PAL/NTSC television |
| BT.709 | High definition (HD) video, e.g., 720p/1080p |

The two standards use different conversion coefficients; mixing them causes color偏差.

## Data Range

Uses limited range:

| Component | Range | Description |
|-----------|-------|-------------|
| Y | 16~235 | Luma, 16 is black, 235 is white |
| U / V | 16~240 | Chroma, 128 is neutral |
| R / G / B | 0~255 | Full range |

## API

### BT.601

```c
void bt601_yuv2rgb(uint8_t y, uint8_t u, uint8_t v, uint8_t * r, uint8_t * g, uint8_t * b);
```

BT.601 YUV to RGB conversion, result clamped to [0, 255].

```c
void bt601_rgb2yuv(uint8_t r, uint8_t g, uint8_t b, uint8_t * y, uint8_t * u, uint8_t * v);
```

BT.601 RGB to YUV conversion.

### BT.709

```c
void bt709_yuv2rgb(uint8_t y, uint8_t u, uint8_t v, uint8_t * r, uint8_t * g, uint8_t * b);
```

BT.709 YUV to RGB conversion, result clamped to [0, 255].

```c
void bt709_rgb2yuv(uint8_t r, uint8_t g, uint8_t b, uint8_t * y, uint8_t * u, uint8_t * v);
```

BT.709 RGB to YUV conversion.

## Usage Examples

### Decoding Video Frame Pixels

```c
uint8_t y = yplane[i];
uint8_t u = uplane[i / 2];
uint8_t v = vplane[i / 2];
uint8_t r, g, b;
bt709_yuv2rgb(y, u, v, &r, &g, &b);
```

### Encoding RGB to YUV

```c
uint8_t y, u, v;
bt601_rgb2yuv(r, g, b, &y, &u, &v);
```
