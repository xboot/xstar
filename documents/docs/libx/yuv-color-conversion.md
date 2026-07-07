# YUV 色彩转换 (yuv)

YUV 与 RGB 色彩空间转换模块，支持 BT.601 和 BT.709 两种标准，使用整数运算避免浮点依赖。

## 色彩空间

| 标准 | 适用场景 |
|------|---------|
| BT.601 | 标清（SD）视频，如 PAL/NTSC 电视 |
| BT.709 | 高清（HD）视频，如 720p/1080p |

两者使用不同的转换系数，混用会导致色彩偏差。

## 数据范围

采用有限范围（Limited Range）：

| 分量 | 范围 | 说明 |
|------|------|------|
| Y | 16~235 | 亮度，16 为黑，235 为白 |
| U / V | 16~240 | 色度，128 为无色 |
| R / G / B | 0~255 | 全范围 |

## API

### BT.601

```c
void bt601_yuv2rgb(uint8_t y, uint8_t u, uint8_t v, uint8_t * r, uint8_t * g, uint8_t * b);
```

BT.601 标准下 YUV 转 RGB，结果钳位到 [0, 255]。

```c
void bt601_rgb2yuv(uint8_t r, uint8_t g, uint8_t b, uint8_t * y, uint8_t * u, uint8_t * v);
```

BT.601 标准下 RGB 转 YUV。

### BT.709

```c
void bt709_yuv2rgb(uint8_t y, uint8_t u, uint8_t v, uint8_t * r, uint8_t * g, uint8_t * b);
```

BT.709 标准下 YUV 转 RGB，结果钳位到 [0, 255]。

```c
void bt709_rgb2yuv(uint8_t r, uint8_t g, uint8_t b, uint8_t * y, uint8_t * u, uint8_t * v);
```

BT.709 标准下 RGB 转 YUV。

## 使用示例

### 解码视频帧像素

```c
uint8_t y = yplane[i];
uint8_t u = uplane[i / 2];
uint8_t v = vplane[i / 2];
uint8_t r, g, b;
bt709_yuv2rgb(y, u, v, &r, &g, &b);
```

### 编码 RGB 为 YUV

```c
uint8_t y, u, v;
bt601_rgb2yuv(r, g, b, &y, &u, &v);
```
