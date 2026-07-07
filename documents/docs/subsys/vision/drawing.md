# 绘图 (drawing)

在 Vision 图像上绘制基本图形和文字。

## API

| 函数 | 说明 |
|------|------|
| `vision_rectangle(v, x, y, w, h, thickness, c)` | 绘制矩形 |
| `vision_text(v, x, y, wrap, family, style, size, c, fmt, ...)` | 渲染文本 |
| `vision_icon(v, x, y, family, size, code, c)` | 渲染图标符号 |

## 说明

| 函数 | 说明 |
|------|------|
| `vision_rectangle` | 绘制矩形边框，`thickness` 为线宽 |
| `vision_text` | 在指定位置渲染文本，支持换行、字体样式和大小 |
| `vision_icon` | 从字体文件渲染图标符号（如 FontAwesome） |
