#ifndef __XSTAR_LIBX_BREATHING_H__
#define __XSTAR_LIBX_BREATHING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

/*
 * Breathing light: current time (ms) -> brightness [0, 1000]
 */
int breathing(int ms);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_BREATHING_H__ */
