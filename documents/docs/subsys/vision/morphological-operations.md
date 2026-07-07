# 形态学运算 (morphological-operations)

## API

| 函数 | 说明 |
|------|------|
| `vision_dilate(v, times)` | 膨胀 |
| `vision_erode(v, times)` | 腐蚀 |

## 说明

膨胀使亮区域扩张，腐蚀使亮区域收缩。`times` 控制应用次数，多次应用效果累加。用于去噪、边缘检测和形态学开闭运算。
