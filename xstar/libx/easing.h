#ifndef __XSTAR_LIBX_EASING_H__
#define __XSTAR_LIBX_EASING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

struct bezier_t {
	float ax, bx, cx;
	float ay, by, cy;
};

void bezier_init(struct bezier_t * b, float x1, float y1, float x2, float y2);
float bezier_calc(struct bezier_t * b, float t);

struct easing_t {
	struct bezier_t bezier;
	float start;
	float stop;
	float duration;
	float acc;
};

void easing_init(struct easing_t * e, float start, float stop, float duration, float x1, float y1, float x2, float y2);
float easing_calc(struct easing_t * e, float t);
float easing_step(struct easing_t * e, float dt);
int easing_finished(struct easing_t * e);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_EASING_H__ */
