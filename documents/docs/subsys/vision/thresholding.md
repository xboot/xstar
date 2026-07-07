# 阈值处理 (thresholding)

## API

| 函数 | 说明 |
|------|------|
| `vision_threshold(v, threshold, type)` | 阈值化 |
| `vision_inrange(v, lower, upper)` | 范围检测，返回二值图像 |

## 阈值类型

| 类型 | 说明 |
|------|------|
| `"binary"` | 像素 > threshold → 255，否则 0 |
| `"binary-invert"` | 像素 > threshold → 0，否则 255 |
| `"tozero"` | 像素 ≤ threshold → 0，否则不变 |
| `"tozero-invert"` | 像素 > threshold → 0，否则不变 |
| `"truncate"` | 像素 > threshold → threshold，否则不变 |

## 说明

当 `threshold < 0` 或 `> 255` 时，自动使用 **Otsu 方法** 计算最优阈值。

`vision_inrange` 返回新分配的灰度图，像素在 `[lower, upper]` 范围内为 255，否则为 0。常用于颜色检测和图像分割。
