# 颜色 (color)

## 结构体

```c
struct color_t {
    uint8_t r;  /* 红 */
    uint8_t g;  /* 绿 */
    uint8_t b;  /* 蓝 */
    uint8_t a;  /* 透明度 */
};
```

像素格式为 32 位预乘 ARGB，native-endian。

## API

| 函数 | 说明 |
|------|------|
| `color_init(c, r, g, b, a)` | 初始化颜色 |
| `color_init_string(c, s)` | 从字符串解析颜色 |
| `color_random(c)` | 生成随机颜色 |
| `color_get_premult(c)` | 获取预乘 ARGB 值 |
| `color_set_premult(c, v)` | 从预乘 ARGB 设置 |
| `color_luminance(c)` | 计算亮度 (0.3R + 0.59G + 0.11B) |
| `color_mix(c, a, b, weight)` | 混合两种颜色 |
| `color_lighten(c, o, level)` | 变亮 |
| `color_darken(c, o, level)` | 变暗 |
| `color_level(c, o, level)` | 电平调整 [-10, +10] |
| `color_set_hsva(c, h, s, v, a)` | 从 HSVA 设置 [0-1] |
| `color_get_hsva(c, h, s, v, a)` | 获取 HSVA [0-1] |
| `color_premult_blend(dst, len, color, alpha)` | 预乘混合 |

## 字符串解析格式

支持以下格式：

| 格式 | 示例 |
|------|------|
| `#RGB` | `#F00` = 红色 |
| `#RGBA` | `#F008` = 半透明红 |
| `#RRGGBB` | `#FF0000` = 红色 |
| `#RRGGBBAA` | `#FF000080` = 半透明红 |
| `r,g,b,a` | `255,0,0,128` = 半透明红 |
| CSS 名称 | `"red"`、`"blue"` 等 |
