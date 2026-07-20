# Vector Graphics (cg)

A Cairo-like 2D vector graphics engine providing path construction, affine transforms, filling, stroking, clipping, gradients, and textures. Located at `xstar/kernel/cg/`, with an integrated FreeType outline rasterizer (xft). Integrates with Surface objects via `surface_get_cg_ctx()`.

## Data Types

### Basic Types

```c
struct cg_point_t { float x; float y; };
struct cg_rect_t  { float x; float y; float w; float h; };
struct cg_color_t { float r; float g; float b; float a; };  /* RGBA, [0, 1] */
struct cg_matrix_t { float a; float b; float c; float d; float tx; float ty; };
struct cg_gradient_stop_t { float offset; struct cg_color_t color; };  /* offset [0, 1] */
```

`cg_matrix_t` represents a 2D affine transform matrix `[a b 0; c d 0; tx ty 1]`.

### Surface

```c
struct cg_surface_t {
    int refcnt;
    int width;
    int height;
    int stride;
    int owndata;
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

## Enums

| Enum | Value | Description |
|------|-------|-------------|
| `CG_LINE_CAP_BUTT` | 0 | Butt cap |
| `CG_LINE_CAP_ROUND` | 1 | Round cap |
| `CG_LINE_CAP_SQUARE` | 2 | Square cap |
| `CG_LINE_JOIN_MITER` | 0 | Miter join |
| `CG_LINE_JOIN_ROUND` | 1 | Round join |
| `CG_LINE_JOIN_BEVEL` | 2 | Bevel join |
| `CG_FILL_RULE_NON_ZERO` | 0 | Non-zero winding rule |
| `CG_FILL_RULE_EVEN_ODD` | 1 | Even-odd rule |
| `CG_OPERATOR_CLEAR` | 0 | Clear destination |
| `CG_OPERATOR_SRC` | 1 | Replace destination with source |
| `CG_OPERATOR_DST` | 2 | Keep destination |
| `CG_OPERATOR_SRC_OVER` | 3 | Source over destination (default) |
| `CG_OPERATOR_DST_OVER` | 4 | Destination over source |
| `CG_OPERATOR_SRC_IN` | 5 | Intersection, keep source |
| `CG_OPERATOR_DST_IN` | 6 | Intersection, keep destination |
| `CG_OPERATOR_SRC_OUT` | 7 | Difference, keep source |
| `CG_OPERATOR_DST_OUT` | 8 | Difference, keep destination |
| `CG_OPERATOR_SRC_ATOP` | 9 | Source atop destination |
| `CG_OPERATOR_DST_ATOP` | 10 | Destination atop source |
| `CG_OPERATOR_XOR` | 11 | XOR |
| `CG_SPREAD_METHOD_PAD` | 0 | Pad with edge color |
| `CG_SPREAD_METHOD_REFLECT` | 1 | Mirror reflect |
| `CG_SPREAD_METHOD_REPEAT` | 2 | Repeat |
| `CG_GRADIENT_TYPE_LINEAR` | 0 | Linear gradient |
| `CG_GRADIENT_TYPE_RADIAL` | 1 | Radial gradient |
| `CG_TEXTURE_TYPE_PLAIN` | 0 | Plain texture |
| `CG_TEXTURE_TYPE_TILED` | 1 | Tiled texture |
| `CG_PAINT_TYPE_COLOR` | 0 | Solid color |
| `CG_PAINT_TYPE_GRADIENT` | 1 | Gradient |
| `CG_PAINT_TYPE_TEXTURE` | 2 | Texture |

## Surface Management

| Function | Description |
|------|------|
| `cg_surface_create(width, height)` | Create surface (allocates pixel memory) |
| `cg_surface_create_for_data(width, height, pixels)` | Create surface (uses external memory) |
| `cg_surface_destroy(surface)` | Release reference (freed when refcount reaches zero) |
| `cg_surface_reference(surface)` | Increment reference count |

## Paint

Paint is a reusable drawing source supporting solid color, gradient, and texture types, managed via reference counting.

| Function | Description |
|------|------|
| `cg_paint_create_rgb(r, g, b)` | Create solid color paint (opaque) |
| `cg_paint_create_rgba(r, g, b, a)` | Create solid color paint (with alpha) |
| `cg_paint_create_color(color)` | Create solid color paint from `cg_color_t` |
| `cg_paint_create_linear_gradient(x1, y1, x2, y2, spread, stops, nstops, m)` | Create linear gradient paint |
| `cg_paint_create_radial_gradient(cx0, cy0, r0, cx1, cy1, r1, spread, stops, nstops, m)` | Create radial gradient paint |
| `cg_paint_create_texture(surface, type, opacity, m)` | Create texture paint |
| `cg_paint_destroy(paint)` | Release reference |
| `cg_paint_reference(paint)` | Increment reference count |

Gradient `stops` is an array of `cg_gradient_stop_t` with `offset` in [0, 1], sorted ascending. `m` is an optional transform matrix; pass `NULL` for identity.

## Path

Paths are constructed from move_to / line_to / quad_to / cubic_to / close commands. They can be created independently and added to a context via `cg_add_path()`.

| Function | Description |
|------|------|
| `cg_path_create()` | Create empty path |
| `cg_path_destroy(path)` | Destroy path |
| `cg_path_reference(path)` | Increment reference count |
| `cg_path_reset(path)` | Clear path |
| `cg_path_move_to(path, x, y)` | Move to point |
| `cg_path_line_to(path, x, y)` | Line to point |
| `cg_path_quad_to(path, x1, y1, x2, y2)` | Quadratic Bezier curve |
| `cg_path_cubic_to(path, x1, y1, x2, y2, x3, y3)` | Cubic Bezier curve |
| `cg_path_arc_to(path, rx, ry, angle, large, sweep, x, y)` | Elliptical arc to point |
| `cg_path_close(path)` | Close sub-path |
| `cg_path_sub_path(path)` | End current sub-path (without closing) |
| `cg_path_add_rectangle(path, x, y, w, h)` | Add rectangle |
| `cg_path_add_round_rectangle(path, x, y, w, h, rx, ry)` | Add rounded rectangle |
| `cg_path_add_ellipse(path, cx, cy, rx, ry)` | Add ellipse |
| `cg_path_add_circle(path, cx, cy, r)` | Add circle |
| `cg_path_add_arc(path, cx, cy, r, a0, a1, ccw)` | Add arc |
| `cg_path_add_path(path, source, m)` | Append another path (with optional transform) |
| `cg_path_transform(path, m)` | Apply matrix transform to path |
| `cg_path_get_current_point(path, x, y)` | Get current point |
| `cg_path_extents(path, extents, tight)` | Get path bounding box |

## Drawing Context

### Lifecycle and State

| Function | Description |
|------|------|
| `cg_create(surface)` | Create context |
| `cg_destroy(ctx)` | Destroy context |
| `cg_save(ctx)` | Save current state (push to stack) |
| `cg_restore(ctx)` | Restore last saved state (pop from stack) |

`cg_save()` / `cg_restore()` use stack-based management. Saved state includes: source paint, color, transform matrix, stroke style, dash pattern, clip region, fill rule, operator, and opacity.

### Source

| Function | Description |
|------|------|
| `cg_set_source(ctx, paint)` | Set paint object |
| `cg_set_source_rgb(ctx, r, g, b)` | Set solid color source (opaque) |
| `cg_set_source_rgba(ctx, r, g, b, a)` | Set solid color source (with alpha) |
| `cg_set_source_surface(ctx, surface, x, y)` | Set texture source at position |
| `cg_set_linear_gradient(ctx, x1, y1, x2, y2, spread, stops, nstops, m)` | Set linear gradient source |
| `cg_set_radial_gradient(ctx, cx0, cy0, r0, cx1, cy1, r1, spread, stops, nstops, m)` | Set radial gradient source |
| `cg_set_texture(ctx, surface, type, opacity, m)` | Set texture source |
| `cg_get_source(ctx, color)` | Get current source color |

### Operator and Opacity

| Function | Description |
|------|------|
| `cg_set_operator(ctx, op)` | Set compositing operator |
| `cg_get_operator(ctx)` | Get current operator |
| `cg_set_opacity(ctx, opacity)` | Set global opacity [0, 1] |
| `cg_get_opacity(ctx)` | Get global opacity |
| `cg_set_fill_rule(ctx, rule)` | Set fill rule |

### Stroke Settings

| Function | Description |
|------|------|
| `cg_set_line_width(ctx, width)` | Set line width |
| `cg_set_line_cap(ctx, cap)` | Set line cap style |
| `cg_set_line_join(ctx, join)` | Set line join style |
| `cg_set_miter_limit(ctx, limit)` | Set miter limit |
| `cg_set_dash(ctx, dashes, ndashes, offset)` | Set dash pattern |
| `cg_set_dash_array(ctx, dashes, ndashes)` | Set dash array |
| `cg_set_dash_offset(ctx, offset)` | Set dash offset |

### Transform

| Function | Description |
|------|------|
| `cg_translate(ctx, tx, ty)` | Append translation |
| `cg_scale(ctx, sx, sy)` | Append scale |
| `cg_shear(ctx, shx, shy)` | Append shear |
| `cg_rotate(ctx, angle)` | Append rotation (radians) |
| `cg_transform(ctx, m)` | Append matrix |
| `cg_set_matrix(ctx, m)` | Replace current matrix |
| `cg_identity_matrix(ctx)` | Reset to identity matrix |
| `cg_get_matrix(ctx)` | Get current matrix |

### Path Construction

Path construction functions on the context operate on the internal path. The path is automatically cleared after drawing (use `_preserve` variants to keep it).

| Function | Description |
|------|------|
| `cg_new_path(ctx)` | Clear current path |
| `cg_new_sub_path(ctx)` | Start new sub-path (without closing current) |
| `cg_close_path(ctx)` | Close current sub-path |
| `cg_move_to(ctx, x, y)` | Move to |
| `cg_line_to(ctx, x, y)` | Line to |
| `cg_quad_to(ctx, x1, y1, x2, y2)` | Quadratic Bezier |
| `cg_cubic_to(ctx, x1, y1, x2, y2, x3, y3)` | Cubic Bezier |
| `cg_arc_to(ctx, rx, ry, angle, large, sweep, x, y)` | Elliptical arc to |
| `cg_rel_move_to(ctx, dx, dy)` | Relative move to |
| `cg_rel_line_to(ctx, dx, dy)` | Relative line to |
| `cg_rel_quad_to(ctx, dx1, dy1, dx2, dy2)` | Relative quadratic Bezier |
| `cg_rel_cubic_to(ctx, dx1, dy1, dx2, dy2, dx3, dy3)` | Relative cubic Bezier |
| `cg_rel_arc_to(ctx, rx, ry, angle, large, sweep, dx, dy)` | Relative elliptical arc to |
| `cg_rectangle(ctx, x, y, w, h)` | Rectangle |
| `cg_round_rectangle(ctx, x, y, w, h, rx, ry)` | Rounded rectangle |
| `cg_ellipse(ctx, cx, cy, rx, ry)` | Ellipse |
| `cg_circle(ctx, cx, cy, r)` | Circle |
| `cg_arc(ctx, cx, cy, r, a0, a1)` | Arc (clockwise) |
| `cg_arc_negative(ctx, cx, cy, r, a0, a1)` | Arc (counterclockwise) |
| `cg_add_path(ctx, path)` | Append standalone path |

### Rendering

| Function | Description |
|------|------|
| `cg_fill(ctx)` | Fill current path, then clear path |
| `cg_fill_preserve(ctx)` | Fill current path, preserve path |
| `cg_stroke(ctx)` | Stroke current path, then clear path |
| `cg_stroke_preserve(ctx)` | Stroke, preserve path |
| `cg_paint(ctx)` | Fill entire clip region with current source |
| `cg_paint_with_alpha(ctx, alpha)` | Fill entire clip region with specified alpha |
| `cg_mask(ctx, paint)` | Use paint as mask |
| `cg_mask_surface(ctx, mask, x, y)` | Use surface as mask |

### Clipping

| Function | Description |
|------|------|
| `cg_reset_clip(ctx)` | Reset clip to entire surface |
| `cg_clip(ctx)` | Set current path as clip, then clear path |
| `cg_clip_preserve(ctx)` | Intersect current path with existing clip, preserve path |

Clips can be stacked: multiple `cg_clip()` calls intersect with the existing clip region. Use `cg_save()` / `cg_restore()` to save and restore clip state.

### Queries

| Function | Description |
|------|------|
| `cg_get_surface(ctx)` | Get bound surface |
| `cg_get_path(ctx)` | Get current path object |
| `cg_get_current_point(ctx, x, y)` | Get current point coordinates |
| `cg_has_current_point(ctx)` | Whether a current point exists |
| `cg_in_fill(ctx, x, y)` | Whether point is inside fill region |
| `cg_in_stroke(ctx, x, y)` | Whether point is inside stroke region |
| `cg_in_clip(ctx, x, y)` | Whether point is inside clip region |
| `cg_fill_extents(ctx, extents)` | Get fill region bounding box |
| `cg_stroke_extents(ctx, extents)` | Get stroke region bounding box |
| `cg_clip_extents(ctx, extents)` | Get clip region bounding box |

## Matrix Operations

Matrix functions are independent of the context and operate directly on `cg_matrix_t` structs.

| Function | Description |
|------|------|
| `cg_matrix_init(m, a, b, c, d, tx, ty)` | Initialize matrix |
| `cg_matrix_init_identity(m)` | Identity matrix |
| `cg_matrix_init_translate(m, tx, ty)` | Translation matrix |
| `cg_matrix_init_scale(m, sx, sy)` | Scale matrix |
| `cg_matrix_init_rotate(m, r)` | Rotation matrix (radians) |
| `cg_matrix_init_shear(m, shx, shy)` | Shear matrix |
| `cg_matrix_translate(m, tx, ty)` | Append translation |
| `cg_matrix_scale(m, sx, sy)` | Append scale |
| `cg_matrix_rotate(m, r)` | Append rotation |
| `cg_matrix_shear(m, shx, shy)` | Append shear |
| `cg_matrix_multiply(m, m1, m2)` | Multiply matrices m = m1 × m2 |
| `cg_matrix_invert(m)` | Invert (returns 1 on success, 0 for singular matrix) |

## Surface Integration

`cg` integrates with the graphics subsystem via the `cg_render_t` field in `surface_t`. The context is lazily created on first access:

| Function | Description |
|------|------|
| `surface_get_cg_surface(s)` | Get the `cg_surface_t` associated with a Surface (lazy init) |
| `surface_get_cg_ctx(s)` | Get the `cg_ctx_t` associated with a Surface (lazy init) |

`cg_surface_create_for_data()` uses the Surface's pixel memory directly with no copy. `surface_blit()` and `surface_fill()` automatically fall back to cg rendering when G2D hardware acceleration is unavailable.

## Default State

| Property | Default Value |
|------|--------|
| Color | Opaque black (0, 0, 0, 1) |
| Transform matrix | Identity |
| Line width | 1.0 |
| Line cap | `CG_LINE_CAP_BUTT` |
| Line join | `CG_LINE_JOIN_MITER` |
| Miter limit | 10.0 |
| Fill rule | `CG_FILL_RULE_NON_ZERO` |
| Operator | `CG_OPERATOR_SRC_OVER` |
| Opacity | 1.0 |
| Clip | None (entire surface) |

## Usage Example

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

Standalone usage (without Surface):

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

Linear gradient example:

```c
struct cg_gradient_stop_t stops[] = {
    { 0.0f, { 1.0f, 0.0f, 0.0f, 1.0f } },  /* red */
    { 0.5f, { 0.0f, 1.0f, 0.0f, 1.0f } },  /* green */
    { 1.0f, { 0.0f, 0.0f, 1.0f, 1.0f } },  /* blue */
};
cg_set_linear_gradient(ctx, 0, 0, 200, 0, CG_SPREAD_METHOD_PAD, stops, 3, NULL);
cg_rectangle(ctx, 0, 0, 200, 100);
cg_fill(ctx);
```
