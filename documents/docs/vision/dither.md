# 抖动 (dither)

误差扩散抖动，用于将图像减少到较少颜色。

## API

| 函数 | 说明 |
|------|------|
| `vision_dither(v, type)` | 误差扩散抖动 |
| `vision_dither_palette(v, palette, n)` | 自定义调色板 Floyd-Steinberg 抖动 |

## 抖动算法

| 类型 | 说明 |
|------|------|
| `"floyd-steinberg"` | Floyd-Steinberg 误差扩散 |
| `"jarvis-judice-ninke"` | Jarvis-Judice-Ninke 误差扩散 |
| `"stucki"` | Stucki 误差扩散 |
| `"atkinson"` | Atkinson 误差扩散 |

## 说明

`vision_dither` 仅适用于灰度图像，误差向相邻像素扩散。

`vision_dither_palette` 使用 Floyd-Steinberg 配合自定义调色板：
- 灰度图默认使用 2 色（黑/白）调色板
- RGB 图默认使用 8 标准色调色板
