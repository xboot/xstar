#ifndef __XSTAR_DRIVER_INPUT_RC_CORE_H__
#define __XSTAR_DRIVER_INPUT_RC_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <driver/input/input.h>
#include <driver/input/rc/rc-decoder-nec.h>

struct rc_map_t {
	uint32_t scancode;
	uint32_t keycode;
};

struct rc_decoder_t {
	struct rc_decoder_nec_t nec;
	struct rc_map_t * map;
	int size;
};

uint32_t rc_decoder_handle(struct rc_decoder_t * decoder, int pulse, int duration);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_INPUT_RC_CORE_H__ */
