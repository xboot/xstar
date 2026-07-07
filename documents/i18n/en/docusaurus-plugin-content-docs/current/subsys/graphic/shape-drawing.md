# Shape Drawing (shape-drawing)

Vector drawing operations via the render backend.

## State Management

| Function | Description |
|------|------|
| `surface_shape_save(s)` | Save drawing state |
| `surface_shape_restore(s)` | Restore drawing state |
| `surface_shape_set_source(s, o, x, y)` | Set source pattern |
| `surface_shape_set_source_color(s, c)` | Set source color |
| `surface_shape_set_fill_rule(s, t)` | Set fill rule |
| `surface_shape_set_line_width(s, w)` | Set line width |
| `surface_shape_set_line_cap(s, t)` | Set line cap style |
| `surface_shape_set_line_join(s, t)` | Set line join style |
| `surface_shape_set_miter_limit(s, l)` | Set miter limit |
| `surface_shape_set_dash(s, dashes, n, offset)` | Set dash pattern |

## Path Construction

| Function | Description |
|------|------|
| `surface_shape_new_path(s)` | New path |
| `surface_shape_close_path(s)` | Close path |
| `surface_shape_move_to(s, x, y)` | Move to |
| `surface_shape_line_to(s, x, y)` | Line to |
| `surface_shape_curve_to(s, x1, y1, x2, y2, x3, y3)` | Cubic Bézier curve |
| `surface_shape_rectangle(s, x, y, w, h)` | Rectangle |
| `surface_shape_round_rectangle(s, x, y, w, h, r)` | Rounded rectangle |
| `surface_shape_arc(s, cx, cy, r, a0, a1)` | Arc |
| `surface_shape_arc_negative(s, cx, cy, r, a0, a1)` | Negative arc |
| `surface_shape_circle(s, cx, cy, r)` | Circle |
| `surface_shape_ellipse(s, cx, cy, rx, ry)` | Ellipse |

## Transform

| Function | Description |
|------|------|
| `surface_shape_translate(s, tx, ty)` | Translate |
| `surface_shape_scale(s, sx, sy)` | Scale |
| `surface_shape_rotate(s, r)` | Rotate |
| `surface_shape_transform(s, m)` | Apply matrix |
| `surface_shape_set_matrix(s, m)` | Set matrix |
| `surface_shape_identity_matrix(s)` | Reset matrix |

## Rendering

| Function | Description |
|------|------|
| `surface_shape_clip(s)` | Clip |
| `surface_shape_clip_preserve(s)` | Clip and preserve path |
| `surface_shape_fill(s)` | Fill |
| `surface_shape_fill_preserve(s)` | Fill and preserve path |
| `surface_shape_stroke(s)` | Stroke |
| `surface_shape_stroke_preserve(s)` | Stroke and preserve path |
| `surface_shape_paint(s)` | Paint |

## String Parameter Values

| Parameter | Allowed values |
|------|--------|
| Fill rule | `"even-odd"`, `"winding"` |
| Line cap | `"butt"`, `"round"`, `"square"` |
| Line join | `"miter"`, `"round"`, `"bevel"` |
