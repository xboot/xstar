# Shape绘图 (shape-drawing)

通过 render 后端进行矢量绘图操作。

## 状态管理

| 函数 | 说明 |
|------|------|
| `surface_shape_save(s)` | 保存绘图状态 |
| `surface_shape_restore(s)` | 恢复绘图状态 |
| `surface_shape_set_source(s, o, x, y)` | 设置源图案 |
| `surface_shape_set_source_color(s, c)` | 设置源颜色 |
| `surface_shape_set_fill_rule(s, t)` | 设置填充规则 |
| `surface_shape_set_line_width(s, w)` | 设置线宽 |
| `surface_shape_set_line_cap(s, t)` | 设置线端形状 |
| `surface_shape_set_line_join(s, t)` | 设置线连接形状 |
| `surface_shape_set_miter_limit(s, l)` | 设置斜接限制 |
| `surface_shape_set_dash(s, dashes, n, offset)` | 设置虚线 |

## 路径构建

| 函数 | 说明 |
|------|------|
| `surface_shape_new_path(s)` | 新建路径 |
| `surface_shape_close_path(s)` | 闭合路径 |
| `surface_shape_move_to(s, x, y)` | 移动到 |
| `surface_shape_line_to(s, x, y)` | 画线到 |
| `surface_shape_curve_to(s, x1, y1, x2, y2, x3, y3)` | 三次贝塞尔曲线 |
| `surface_shape_rectangle(s, x, y, w, h)` | 矩形 |
| `surface_shape_round_rectangle(s, x, y, w, h, r)` | 圆角矩形 |
| `surface_shape_arc(s, cx, cy, r, a0, a1)` | 圆弧 |
| `surface_shape_arc_negative(s, cx, cy, r, a0, a1)` | 逆圆弧 |
| `surface_shape_circle(s, cx, cy, r)` | 圆形 |
| `surface_shape_ellipse(s, cx, cy, rx, ry)` | 椭圆 |

## 变换

| 函数 | 说明 |
|------|------|
| `surface_shape_translate(s, tx, ty)` | 平移 |
| `surface_shape_scale(s, sx, sy)` | 缩放 |
| `surface_shape_rotate(s, r)` | 旋转 |
| `surface_shape_transform(s, m)` | 应用矩阵 |
| `surface_shape_set_matrix(s, m)` | 设置矩阵 |
| `surface_shape_identity_matrix(s)` | 重置矩阵 |

## 渲染

| 函数 | 说明 |
|------|------|
| `surface_shape_clip(s)` | 裁剪 |
| `surface_shape_clip_preserve(s)` | 裁剪并保留路径 |
| `surface_shape_fill(s)` | 填充 |
| `surface_shape_fill_preserve(s)` | 填充并保留路径 |
| `surface_shape_stroke(s)` | 描边 |
| `surface_shape_stroke_preserve(s)` | 描边并保留路径 |
| `surface_shape_paint(s)` | 绘制 |

## 字符串参数取值

| 参数 | 可选值 |
|------|--------|
| 填充规则 | `"even-odd"`、`"winding"` |
| 线端形状 | `"butt"`、`"round"`、`"square"` |
| 线连接 | `"miter"`、`"round"`、`"bevel"` |
