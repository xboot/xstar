# 颜色转换 (color-conversion)

## API

| 函数 | 说明 |
|------|------|
| `vision_gray(v)` | 灰度化 |
| `vision_sepia(v)` | 怀旧色调 |
| `vision_invert(v)` | 颜色反转 |
| `vision_gamma(v, gamma)` | 伽马校正 |
| `vision_colormap(v, type)` | 色彩映射 |

## 色彩映射类型

| 类型 | 说明 |
|------|------|
| `"parula"` | Parula 配色（默认） |
| `"jet"` | Jet 配色 |
| `"rainbow"` | 彩虹配色 |

## 说明

`vision_gray` 使用亮度公式 `0.299R + 0.587G + 0.114B` 转换 RGB 为灰度。
`vision_colormap` 应用于 RGB 图像，每个通道值通过色彩映射表查找映射。
