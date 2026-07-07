#ifndef __XSTAR_LIBX_TSFILTER_H__
#define __XSTAR_LIBX_TSFILTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <libx/median.h>
#include <libx/mean.h>

struct tsfilter_t {
	struct median_filter_t * mx, * my;
	struct mean_filter_t * nx, * ny;
	int cal[7];
};

struct tsfilter_t * tsfilter_alloc(int ml, int nl);
void tsfilter_free(struct tsfilter_t * filter);
void tsfilter_setcal(struct tsfilter_t * filter, int * cal);
void tsfilter_update(struct tsfilter_t * filter, int * x, int * y);
void tsfilter_clear(struct tsfilter_t * filter);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_TSFILTER_H__ */
