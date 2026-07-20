#ifndef __XSTAR_DRIVER_G2D_H__
#define __XSTAR_DRIVER_G2D_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>
#include <kernel/graphic/surface.h>

struct surface_t;

struct g2d_t {
	char * name;

	int (*blit)(struct g2d_t * g2d, struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, struct surface_t * o);
	int (*fill)(struct g2d_t * g2d, struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, int w, int h, struct color_t * c);
	void * priv;
};

struct g2d_t * search_g2d(const char * name);
struct g2d_t * search_first_g2d(void);
struct device_t * register_g2d(struct g2d_t * g2d, struct driver_t * drv);
void unregister_g2d(struct g2d_t * g2d);

int g2d_blit(struct g2d_t * g2d, struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, struct surface_t * o);
int g2d_fill(struct g2d_t * g2d, struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, int w, int h, struct color_t * c);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_G2D_H__ */
