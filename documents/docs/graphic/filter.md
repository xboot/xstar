# 滤镜 (filter)

Surface 内置的图像滤镜。

## API

| 函数 | 说明 |
|------|------|
| `surface_filter_gray(s)` | 灰度化 |
| `surface_filter_sepia(s)` | 怀旧色调 |
| `surface_filter_invert(s)` | 颜色反转 |
| `surface_filter_coloring(s, c)` | 颜色着色 |
| `surface_filter_gamma(s, gamma)` | 伽马校正 |
| `surface_filter_hue(s, angle)` | 色调旋转（度） |
| `surface_filter_saturate(s, saturate)` | 饱和度调整 [-100, 100] |
| `surface_filter_brightness(s, brightness)` | 亮度调整 [-100, 100] |
| `surface_filter_contrast(s, contrast)` | 对比度调整 [-100, 100] |
| `surface_filter_opacity(s, alpha)` | 不透明度 [0, 100] |
| `surface_filter_haldclut(s, clut, type)` | HALD CLUT 颜色查找表 |
| `surface_filter_blur(s, radius)` | 模糊 |

## 说明

`surface_filter_haldclut` 支持两种插值模式：

| 模式 | 说明 |
|------|------|
| `"nearest"` | 最近邻插值（速度快） |
| `"trilinear"` | 三线性插值（质量好） |

所有滤镜直接修改 Surface 像素数据，不可逆。
