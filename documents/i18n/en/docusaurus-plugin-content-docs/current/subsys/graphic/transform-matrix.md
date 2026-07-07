# Transform Matrix (transform-matrix)

2D affine transformation matrix, represented as a 3x3 matrix: `[a b 0; c d 0; tx ty 1]`.

## Struct

```c
struct matrix2d_t {
    float a; float b;
    float c; float d;
    float tx; float ty;
};
```

## API

| Function | Description |
|------|------|
| `matrix2d_init(m, a, b, c, d, tx, ty)` | Initialize matrix |
| `matrix2d_init_identity(m)` | Identity matrix |
| `matrix2d_init_translate(m, tx, ty)` | Translation |
| `matrix2d_init_scale(m, sx, sy)` | Scaling |
| `matrix2d_init_rotate(m, r)` | Rotation |
| `matrix2d_multiply(m, m1, m2)` | Matrix multiply |
| `matrix2d_invert(m)` | Invert |
| `matrix2d_translate(m, tx, ty)` | Append translation |
| `matrix2d_scale(m, sx, sy)` | Append scaling |
| `matrix2d_rotate(m, r)` | Append rotation |
| `matrix2d_transform_point(m, x, y)` | Transform point |
| `matrix2d_transform_distance(m, dx, dy)` | Transform distance |
| `matrix2d_transform_bounds(m, x1, y1, x2, y2)` | Transform bounds |
| `matrix2d_transform_region(m, w, h, region)` | Transform region |
