# 特效 (visual-effect)

Surface 内置的视觉效果。

## API

| 函数 | 说明 |
|------|------|
| `surface_effect_glass(s, clip, x, y, w, h, radius)` | 毛玻璃模糊效果 |
| `surface_effect_shadow(s, clip, x, y, w, h, radius, c)` | 投影阴影 |
| `surface_effect_gradient(s, clip, x, y, w, h, lt, rt, rb, lb)` | 四角双线性渐变 |
| `surface_effect_checkerboard(s, clip, x, y, w, h)` | 棋盘格透明图案 |

## 说明

| 特效 | 说明 |
|------|------|
| **毛玻璃** | 对指定区域应用模糊效果，模拟磨砂玻璃质感 |
| **投影** | 在区域下方绘制带模糊的彩色阴影 |
| **渐变** | 四个角分别指定颜色，内部双线性插值填充 |
| **棋盘格** | 绘制棋盘格图案用于显示透明区域 |
