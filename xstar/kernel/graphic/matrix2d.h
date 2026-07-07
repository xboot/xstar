#ifndef __XSTAR_KERNEL_GRAPHIC_MATRIX2D_H__
#define __XSTAR_KERNEL_GRAPHIC_MATRIX2D_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/graphic/region.h>

/*
 * | [a]  [b]  [0] |
 * | [c]  [d]  [0] |
 * | [tx] [ty] [1] |
 */
struct matrix2d_t {
    float a; float b;
    float c; float d;
    float tx; float ty;
};

void matrix2d_init(struct matrix2d_t * m, float a, float b, float c, float d, float tx, float ty);
void matrix2d_init_identity(struct matrix2d_t * m);
void matrix2d_init_translate(struct matrix2d_t * m, float tx, float ty);
void matrix2d_init_scale(struct matrix2d_t * m, float sx, float sy);
void matrix2d_init_rotate(struct matrix2d_t * m, float r);
void matrix2d_multiply(struct matrix2d_t * m, struct matrix2d_t * m1, struct matrix2d_t * m2);
void matrix2d_invert(struct matrix2d_t * m);
void matrix2d_translate(struct matrix2d_t * m, float tx, float ty);
void matrix2d_scale(struct matrix2d_t * m, float sx, float sy);
void matrix2d_rotate(struct matrix2d_t * m, float r);
void matrix2d_transform_distance(struct matrix2d_t * m, float * dx, float * dy);
void matrix2d_transform_point(struct matrix2d_t * m, float * x, float * y);
void matrix2d_transform_bounds(struct matrix2d_t * m, float * x1, float * y1, float * x2, float * y2);
void matrix2d_transform_region(struct matrix2d_t * m, float w, float h, struct region_t * region);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_GRAPHIC_MATRIX2D_H__ */
