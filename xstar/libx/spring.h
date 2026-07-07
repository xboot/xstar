#ifndef __XSTAR_LIBX_SPRING_H__
#define __XSTAR_LIBX_SPRING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

struct spring_t {
	float start;
	float stop;
	float velocity;
	float tension;
	float friction;
	float acc;
};

void spring_init(struct spring_t * s, float start, float stop, float velocity, float tension, float friction);
int spring_step(struct spring_t * s, float dt);

static inline float spring_position(struct spring_t * s)
{
	return s->start;
}

static inline float spring_velocity(struct spring_t * s)
{
	return s->velocity;
}

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_SPRING_H__ */
