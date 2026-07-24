# 矢量绘图 (cg)

类 Cairo 的 2D 矢量绘图引擎，提供路径构建、仿射变换、填充、描边、裁剪、渐变和纹理等能力。位于 `xstar/kernel/cg/`，内部集成 FreeType 轮廓光栅化器（xft），通过 `surface_get_cg_ctx()` 与 Surface 对象集成。

## 数据类型

### 基本类型

```c
struct cg_point_t { float x; float y; };
struct cg_rect_t  { float x; float y; float w; float h; };
struct cg_color_t { float r; float g; float b; float a; };  /* RGBA, [0, 1] */
struct cg_matrix_t { float a; float b; float c; float d; float tx; float ty; };
struct cg_gradient_stop_t { float offset; struct cg_color_t color; };  /* offset [0, 1] */
```

`cg_matrix_t` 表示 2D 仿射变换矩阵 `[a b 0; c d 0; tx ty 1]`。

### Surface

```c
struct cg_surface_t {
    int refcnt;
    int width;
    int height;
    int stride;
    int owns;
    unsigned char * pixels;
};
```

### Path

```c
struct cg_path_t {
    int refcnt;
    int num_points;
    int num_contours;
    int num_curves;
    int sub_path;
    struct cg_point_t start_point;
    struct { union cg_path_element_t * data; int size; int capacity; } elements;
};
```

### Paint

```c
struct cg_paint_t          { int refcnt; enum cg_paint_type_t type; };
struct cg_solid_paint_t    { struct cg_paint_t base; struct cg_color_t color; };
struct cg_gradient_paint_t { struct cg_paint_t base; enum cg_gradient_type_t type;
                             enum cg_spread_method_t spread; struct cg_matrix_t matrix;
                             struct cg_gradient_stop_t * stops; int nstops; float values[6]; };
struct cg_texture_paint_t  { struct cg_paint_t base; enum cg_texture_type_t type;
                             float opacity; struct cg_matrix_t matrix; struct cg_surface_t * surface; };
```

### Context

```c
struct cg_ctx_t {
    struct cg_surface_t * surface;
    struct cg_path_t * path;
    struct cg_state_t * state;
    struct cg_state_t * freed_state;
    struct cg_rect_t clip_rect;
    struct cg_span_buffer_t clip_spans;
    struct cg_span_buffer_t fill_spans;
};
```

## 枚举

| 枚举 | 值 | 说明 |
|------|------|------|
| `CG_LINE_CAP_BUTT` | 0 | 平头线端 |
| `CG_LINE_CAP_ROUND` | 1 | 圆头线端 |
| `CG_LINE_CAP_SQUARE` | 2 | 方头线端 |
| `CG_LINE_JOIN_MITER` | 0 | 斜接连接 |
| `CG_LINE_JOIN_ROUND` | 1 | 圆角连接 |
| `CG_LINE_JOIN_BEVEL` | 2 | 斜切连接 |
| `CG_FILL_RULE_NON_ZERO` | 0 | 非零环绕规则 |
| `CG_FILL_RULE_EVEN_ODD` | 1 | 奇偶规则 |
| `CG_OPERATOR_CLEAR` | 0 | 清除目标 |
| `CG_OPERATOR_SRC` | 1 | 用源替换目标 |
| `CG_OPERATOR_DST` | 2 | 保留目标 |
| `CG_OPERATOR_SRC_OVER` | 3 | 源覆盖目标（默认） |
| `CG_OPERATOR_DST_OVER` | 4 | 目标覆盖源 |
| `CG_OPERATOR_SRC_IN` | 5 | 取交集，保留源 |
| `CG_OPERATOR_DST_IN` | 6 | 取交集，保留目标 |
| `CG_OPERATOR_SRC_OUT` | 7 | 取差集，保留源 |
| `CG_OPERATOR_DST_OUT` | 8 | 取差集，保留目标 |
| `CG_OPERATOR_SRC_ATOP` | 9 | 源在目标之上取交集 |
| `CG_OPERATOR_DST_ATOP` | 10 | 目标在源之上取交集 |
| `CG_OPERATOR_XOR` | 11 | 异或 |
| `CG_SPREAD_METHOD_PAD` | 0 | 填充边缘色 |
| `CG_SPREAD_METHOD_REFLECT` | 1 | 镜像反射 |
| `CG_SPREAD_METHOD_REPEAT` | 2 | 重复 |
| `CG_GRADIENT_TYPE_LINEAR` | 0 | 线性渐变 |
| `CG_GRADIENT_TYPE_RADIAL` | 1 | 径向渐变 |
| `CG_TEXTURE_TYPE_PLAIN` | 0 | 普通纹理 |
| `CG_TEXTURE_TYPE_TILED` | 1 | 平铺纹理 |
| `CG_PAINT_TYPE_COLOR` | 0 | 纯色 |
| `CG_PAINT_TYPE_GRADIENT` | 1 | 渐变 |
| `CG_PAINT_TYPE_TEXTURE` | 2 | 纹理 |

## Surface 管理

| 函数 | 说明 |
|------|------|
| `cg_surface_create(width, height)` | 创建 Surface（分配像素内存） |
| `cg_surface_create_for_data(width, height, pixels)` | 创建 Surface（使用外部内存） |
| `cg_surface_destroy(surface)` | 释放引用（引用计数归零时释放） |
| `cg_surface_reference(surface)` | 增加引用计数 |

## 颜料 (Paint)

Paint 是可复用的绘制源，支持纯色、渐变和纹理三种类型，通过引用计数管理生命周期。

| 函数 | 说明 |
|------|------|
| `cg_paint_create_rgb(r, g, b)` | 创建纯色颜料（不透明） |
| `cg_paint_create_rgba(r, g, b, a)` | 创建纯色颜料（带透明度） |
| `cg_paint_create_color(color)` | 从 `cg_color_t` 创建纯色颜料 |
| `cg_paint_create_linear_gradient(x1, y1, x2, y2, spread, stops, nstops, m)` | 创建线性渐变颜料 |
| `cg_paint_create_radial_gradient(cx0, cy0, r0, cx1, cy1, r1, spread, stops, nstops, m)` | 创建径向渐变颜料 |
| `cg_paint_create_texture(surface, type, opacity, m)` | 创建纹理颜料 |
| `cg_paint_destroy(paint)` | 释放引用 |
| `cg_paint_reference(paint)` | 增加引用计数 |

渐变 `stops` 为 `cg_gradient_stop_t` 数组，`offset` 范围 [0, 1]，按升序排列。`m` 为可选变换矩阵，传 `NULL` 使用单位矩阵。

## 路径 (Path)

路径由 move_to / line_to / quad_to / cubic_to / close 等命令构成，可独立创建后通过 `cg_add_path()` 添加到上下文。

| 函数 | 说明 |
|------|------|
| `cg_path_create()` | 创建空路径 |
| `cg_path_destroy(path)` | 释放路径 |
| `cg_path_reference(path)` | 增加引用计数 |
| `cg_path_reset(path)` | 清空路径 |
| `cg_path_move_to(path, x, y)` | 移动到点 |
| `cg_path_line_to(path, x, y)` | 直线到点 |
| `cg_path_quad_to(path, x1, y1, x2, y2)` | 二次贝塞尔曲线 |
| `cg_path_cubic_to(path, x1, y1, x2, y2, x3, y3)` | 三次贝塞尔曲线 |
| `cg_path_arc_to(path, rx, ry, angle, large, sweep, x, y)` | 椭圆弧到点 |
| `cg_path_close(path)` | 闭合子路径 |
| `cg_path_sub_path(path)` | 结束当前子路径（不闭合） |
| `cg_path_add_rectangle(path, x, y, w, h)` | 添加矩形 |
| `cg_path_add_round_rectangle(path, x, y, w, h, rx, ry)` | 添加圆角矩形 |
| `cg_path_add_ellipse(path, cx, cy, rx, ry)` | 添加椭圆 |
| `cg_path_add_circle(path, cx, cy, r)` | 添加圆形 |
| `cg_path_add_arc(path, cx, cy, r, a0, a1, ccw)` | 添加圆弧 |
| `cg_path_add_path(path, source, m)` | 追加另一路径（可带变换） |
| `cg_path_transform(path, m)` | 对路径应用矩阵变换 |
| `cg_path_get_current_point(path, x, y)` | 获取当前点 |
| `cg_path_extents(path, extents, tight)` | 获取路径边界 |

## 绘图上下文 (Context)

### 生命周期与状态

| 函数 | 说明 |
|------|------|
| `cg_create(surface)` | 创建上下文 |
| `cg_destroy(ctx)` | 销毁上下文 |
| `cg_save(ctx)` | 保存当前状态（压栈） |
| `cg_restore(ctx)` | 恢复上次保存的状态（出栈） |

`cg_save()` / `cg_restore()` 使用栈式管理，保存的内容包括：源颜料、颜色、变换矩阵、描边样式、虚线、裁剪区域、填充规则、运算符、不透明度。

### 源设置

| 函数 | 说明 |
|------|------|
| `cg_set_source(ctx, paint)` | 设置颜料对象 |
| `cg_set_source_rgb(ctx, r, g, b)` | 设置纯色源（不透明） |
| `cg_set_source_rgba(ctx, r, g, b, a)` | 设置纯色源（带透明度） |
| `cg_set_source_surface(ctx, surface, x, y)` | 设置纹理源（在指定位置） |
| `cg_set_linear_gradient(ctx, x1, y1, x2, y2, spread, stops, nstops, m)` | 设置线性渐变源 |
| `cg_set_radial_gradient(ctx, cx0, cy0, r0, cx1, cy1, r1, spread, stops, nstops, m)` | 设置径向渐变源 |
| `cg_set_texture(ctx, surface, type, opacity, m)` | 设置纹理源 |
| `cg_get_source(ctx, color)` | 获取当前源颜色 |

### 运算符与不透明度

| 函数 | 说明 |
|------|------|
| `cg_set_operator(ctx, op)` | 设置合成运算符 |
| `cg_get_operator(ctx)` | 获取当前运算符 |
| `cg_set_opacity(ctx, opacity)` | 设置全局不透明度 [0, 1] |
| `cg_get_opacity(ctx)` | 获取全局不透明度 |
| `cg_set_fill_rule(ctx, rule)` | 设置填充规则 |

### 描边设置

| 函数 | 说明 |
|------|------|
| `cg_set_line_width(ctx, width)` | 设置线宽 |
| `cg_set_line_cap(ctx, cap)` | 设置线端样式 |
| `cg_set_line_join(ctx, join)` | 设置线连接样式 |
| `cg_set_miter_limit(ctx, limit)` | 设置斜接限制 |
| `cg_set_dash(ctx, dashes, ndashes, offset)` | 设置虚线模式 |
| `cg_set_dash_array(ctx, dashes, ndashes)` | 设置虚线数组 |
| `cg_set_dash_offset(ctx, offset)` | 设置虚线偏移 |

### 变换

| 函数 | 说明 |
|------|------|
| `cg_translate(ctx, tx, ty)` | 追加平移 |
| `cg_scale(ctx, sx, sy)` | 追加缩放 |
| `cg_shear(ctx, shx, shy)` | 追加剪切 |
| `cg_rotate(ctx, angle)` | 追加旋转（弧度） |
| `cg_transform(ctx, m)` | 追加矩阵 |
| `cg_set_matrix(ctx, m)` | 替换当前矩阵 |
| `cg_identity_matrix(ctx)` | 重置为单位矩阵 |
| `cg_get_matrix(ctx)` | 获取当前矩阵 |

### 路径构建

上下文上的路径构建函数操作内部路径，绘制后路径自动清空（`_preserve` 变体保留）。

| 函数 | 说明 |
|------|------|
| `cg_new_path(ctx)` | 清空当前路径 |
| `cg_new_sub_path(ctx)` | 开始新子路径（不闭合当前） |
| `cg_close_path(ctx)` | 闭合当前子路径 |
| `cg_move_to(ctx, x, y)` | 移动到 |
| `cg_line_to(ctx, x, y)` | 直线到 |
| `cg_quad_to(ctx, x1, y1, x2, y2)` | 二次贝塞尔 |
| `cg_cubic_to(ctx, x1, y1, x2, y2, x3, y3)` | 三次贝塞尔 |
| `cg_arc_to(ctx, rx, ry, angle, large, sweep, x, y)` | 椭圆弧到 |
| `cg_rel_move_to(ctx, dx, dy)` | 相对移动 |
| `cg_rel_line_to(ctx, dx, dy)` | 相对直线 |
| `cg_rel_quad_to(ctx, dx1, dy1, dx2, dy2)` | 相对二次贝塞尔 |
| `cg_rel_cubic_to(ctx, dx1, dy1, dx2, dy2, dx3, dy3)` | 相对三次贝塞尔 |
| `cg_rel_arc_to(ctx, rx, ry, angle, large, sweep, dx, dy)` | 相对椭圆弧到 |
| `cg_rectangle(ctx, x, y, w, h)` | 矩形 |
| `cg_round_rectangle(ctx, x, y, w, h, rx, ry)` | 圆角矩形 |
| `cg_ellipse(ctx, cx, cy, rx, ry)` | 椭圆 |
| `cg_circle(ctx, cx, cy, r)` | 圆形 |
| `cg_arc(ctx, cx, cy, r, a0, a1)` | 圆弧（顺时针） |
| `cg_arc_negative(ctx, cx, cy, r, a0, a1)` | 圆弧（逆时针） |
| `cg_add_path(ctx, path)` | 追加独立路径 |

### 渲染

| 函数 | 说明 |
|------|------|
| `cg_fill(ctx)` | 填充当前路径，并清空路径 |
| `cg_fill_preserve(ctx)` | 填充当前路径，保留路径 |
| `cg_stroke(ctx)` | 沿当前路径描边，并清空路径 |
| `cg_stroke_preserve(ctx)` | 描边，保留路径 |
| `cg_paint(ctx)` | 用当前源填充整个裁剪区域 |
| `cg_paint_with_alpha(ctx, alpha)` | 用指定透明度填充整个裁剪区域 |
| `cg_mask(ctx, paint)` | 用颜料作为蒙版 |
| `cg_mask_surface(ctx, mask, x, y)` | 用 Surface 作为蒙版 |

### 裁剪

| 函数 | 说明 |
|------|------|
| `cg_reset_clip(ctx)` | 重置裁剪区域为整个 Surface |
| `cg_clip(ctx)` | 将当前路径设为裁剪区域，并清空路径 |
| `cg_clip_preserve(ctx)` | 将当前路径与已有裁剪区域取交集，保留路径 |

裁剪区域可叠加：多次调用 `cg_clip()` 会与已有裁剪区域取交集。使用 `cg_save()` / `cg_restore()` 可保存和恢复裁剪状态。

### 查询

| 函数 | 说明 |
|------|------|
| `cg_get_surface(ctx)` | 获取绑定的 Surface |
| `cg_get_path(ctx)` | 获取当前路径对象 |
| `cg_get_current_point(ctx, x, y)` | 获取当前点坐标 |
| `cg_has_current_point(ctx)` | 是否有当前点 |
| `cg_in_fill(ctx, x, y)` | 点是否在填充区域内 |
| `cg_in_stroke(ctx, x, y)` | 点是否在描边区域内 |
| `cg_in_clip(ctx, x, y)` | 点是否在裁剪区域内 |
| `cg_fill_extents(ctx, extents)` | 获取填充区域边界 |
| `cg_stroke_extents(ctx, extents)` | 获取描边区域边界 |
| `cg_clip_extents(ctx, extents)` | 获取裁剪区域边界 |

## 矩阵操作

矩阵函数独立于上下文，可直接操作 `cg_matrix_t` 结构体。

| 函数 | 说明 |
|------|------|
| `cg_matrix_init(m, a, b, c, d, tx, ty)` | 初始化矩阵 |
| `cg_matrix_init_identity(m)` | 单位矩阵 |
| `cg_matrix_init_translate(m, tx, ty)` | 平移矩阵 |
| `cg_matrix_init_scale(m, sx, sy)` | 缩放矩阵 |
| `cg_matrix_init_rotate(m, r)` | 旋转矩阵（弧度） |
| `cg_matrix_init_shear(m, shx, shy)` | 剪切矩阵 |
| `cg_matrix_translate(m, tx, ty)` | 追加平移 |
| `cg_matrix_scale(m, sx, sy)` | 追加缩放 |
| `cg_matrix_rotate(m, r)` | 追加旋转 |
| `cg_matrix_shear(m, shx, shy)` | 追加剪切 |
| `cg_matrix_multiply(m, m1, m2)` | 矩阵相乘 m = m1 × m2 |
| `cg_matrix_invert(m)` | 求逆（成功返回 1，奇异矩阵返回 0） |

## 与 Surface 集成

`cg` 通过 `surface_t` 的 `cg_render_t` 字段与图形子系统集成。首次调用时惰性创建上下文：

| 函数 | 说明 |
|------|------|
| `surface_get_cg_surface(s)` | 获取 Surface 关联的 `cg_surface_t`（惰性创建） |
| `surface_get_cg_ctx(s)` | 获取 Surface 关联的 `cg_ctx_t`（惰性创建） |

`cg_surface_create_for_data()` 直接使用 Surface 的像素内存，无需拷贝。`surface_blit()` 和 `surface_fill()` 在 G2D 硬件加速不可用时自动回退到 cg 渲染。

## 默认状态

| 属性 | 默认值 |
|------|--------|
| 颜色 | 黑色不透明 (0, 0, 0, 1) |
| 变换矩阵 | 单位矩阵 |
| 线宽 | 1.0 |
| 线端 | `CG_LINE_CAP_BUTT` |
| 线连接 | `CG_LINE_JOIN_MITER` |
| 斜接限制 | 10.0 |
| 填充规则 | `CG_FILL_RULE_NON_ZERO` |
| 运算符 | `CG_OPERATOR_SRC_OVER` |
| 不透明度 | 1.0 |
| 裁剪 | 无（整个 Surface） |

## 使用示例

```c
struct surface_t * s = window_get_surface(w);
struct cg_ctx_t * cg = surface_get_cg_ctx(s);

cg_save(cg);
cg_set_source_rgba(cg, 1.0, 1.0, 0.0, 1.0);
cg_set_line_width(cg, 2);
cg_move_to(cg, x - 15, y);
cg_line_to(cg, x + 15, y);
cg_stroke(cg);

cg_set_line_width(cg, 4);
cg_circle(cg, x, y, 15);
cg_stroke(cg);
cg_restore(cg);
```

独立使用（不依赖 Surface）：

```c
struct cg_surface_t * surface = cg_surface_create(640, 480);
struct cg_ctx_t * ctx = cg_create(surface);

cg_set_source_rgb(ctx, 1.0, 0.0, 0.0);
cg_rectangle(ctx, 10, 10, 100, 80);
cg_fill(ctx);

cg_set_source_rgb(ctx, 0.0, 0.0, 1.0);
cg_set_line_width(ctx, 3);
cg_circle(ctx, 320, 240, 50);
cg_stroke(ctx);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

线性渐变示例：

```c
struct cg_gradient_stop_t stops[] = {
    { 0.0f, { 1.0f, 0.0f, 0.0f, 1.0f } },  /* 红 */
    { 0.5f, { 0.0f, 1.0f, 0.0f, 1.0f } },  /* 绿 */
    { 1.0f, { 0.0f, 0.0f, 1.0f, 1.0f } },  /* 蓝 */
};
cg_set_linear_gradient(ctx, 0, 0, 200, 0, CG_SPREAD_METHOD_PAD, stops, 3, NULL);
cg_rectangle(ctx, 0, 0, 200, 100);
cg_fill(ctx);
```
