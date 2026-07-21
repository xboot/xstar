#ifndef __XSTAR_KERNEL_GRAPHIC_COLOR_H__
#define __XSTAR_KERNEL_GRAPHIC_COLOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

struct color_t {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

static inline void color_init(struct color_t * c, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = a;
}

static inline void color_set_premult(struct color_t * c, uint32_t v)
{
	uint8_t a = (v >> 24) & 0xff;

	if(a != 0)
	{
		if(a == 255)
		{
			c->r = (v >> 16) & 0xff;
			c->g = (v >> 8) & 0xff;
			c->b = (v >> 0) & 0xff;
			c->a = a;
		}
		else
		{
			uint32_t ia = (0xff << 15) / a;
			c->r = (((v >> 16) & 0xff) * ia) >> 15;
			c->g = (((v >> 8) & 0xff) * ia) >> 15;
			c->b = (((v >> 0) & 0xff) * ia) >> 15;
			c->a = a;
		}
	}
	else
	{
		c->r = 0;
		c->g = 0;
		c->b = 0;
		c->a = 0;
	}
}

static inline uint32_t color_get_premult(struct color_t * c)
{
	uint8_t a = c->a;

	if(a != 0)
	{
		if(a == 255)
			return (a << 24) | (c->r << 16) | (c->g << 8) | (c->b << 0);
		return (a << 24) | (XDIV255(c->r * a) << 16) | (XDIV255(c->g * a) << 8) | (XDIV255(c->b * a) << 0);
	}
	return 0;
}

static inline void color_premult_blend(uint32_t * dst, int len, uint32_t color, uint8_t alpha)
{
	if((alpha & (color >> 24)) == 255)
	{
		for(int i = 0; i < len; i++)
			dst[i] = color;
	}
	else
	{
		if(alpha != 255)
			color = XBYTEMUL(color, alpha);
		uint8_t ialpha = 255 - (color >> 24);
		for(int i = 0; i < len; i++)
			dst[i] = color + XBYTEMUL(dst[i], ialpha);
	}
}

static inline void color_mix(struct color_t * c, struct color_t * a, struct color_t * b, uint8_t weight)
{
	int u = (weight << 1) - 255;
	int v = XDIV255(a->a - b->a);
	int w = XDIV255(u * v);
	unsigned char wa = ((u * v == -255) ? u : (u + (a->a - b->a)) / (1 + w) + 255) >> 1;
	unsigned char wb = 255 - wa;

	c->r = XDIV255(a->r * wa) + XDIV255(b->r * wb);
	c->g = XDIV255(a->g * wa) + XDIV255(b->g * wb);
	c->b = XDIV255(a->b * wa) + XDIV255(b->b * wb);
	c->a = XDIV255(a->a * weight) + XDIV255(b->a * (255 - weight));
}

static inline void color_lighten(struct color_t * c, struct color_t * o, uint8_t level)
{
	color_mix(c, &(struct color_t){ 0xff, 0xff, 0xff, 0xff }, o, level);
}

static inline void color_darken(struct color_t * c, struct color_t * o, uint8_t level)
{
	color_mix(c, &(struct color_t){ 0x00, 0x00, 0x00, 0xff }, o, level);
}

static inline void color_level(struct color_t * c, struct color_t * o, int level)
{
	level = XCLAMP(level, -10, 10);
	if(level < 0)
		color_mix(c, &(struct color_t){ 0xff, 0xff, 0xff, 0xff }, o, -level * 25);
	else
		color_mix(c, &(struct color_t){ 0x00, 0x00, 0x00, 0xff }, o, level * 25);
}

/*
 * String: [#RGB], [#RGBA], [#RRGGBB], [#RRGGBBAA], [r, g, b, a], [NAME]
 * http://www.w3.org/TR/css3-color/#svg-color
 */
void color_init_string(struct color_t * c, const char * s);

/*
 * Random color
 */
void color_random(struct color_t * c);

/*
 * luminance = 0.3R + 0.59G + 0.11B
 */
uint8_t color_luminance(struct color_t * c);

/*
 * h, s, v and a from 0.0 to 1.0
 */
void color_set_hsva(struct color_t * c, float h, float s, float v, float a);
void color_get_hsva(struct color_t * c, float * h, float * s, float * v, float * a);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_GRAPHIC_COLOR_H__ */
