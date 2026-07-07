#ifndef __XSTAR_DRIVER_AUDIO_H__
#define __XSTAR_DRIVER_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

struct audio_frame_t {
	/*
	 * The audio sample rate.
	 */
	int rate;

	/*
	 * The number of channel.
	 */
	int channel;

	/*
	 * The number of audio frames, about ~5ms worth of samples at a time.
	 */
	int frames;

	/*
	 * The samples are interleaved.
	 */
	float * samples;
};

static inline int audio_frame_is_valid(struct audio_frame_t * af)
{
	return (af && (af->frames > 0)) ? 1 : 0;
}

static inline int audio_frame_total_time(struct audio_frame_t * af)
{
	return (af && (af->rate > 0)) ? 1000 * af->frames / af->rate : 0;
}

/*
 * [0, 1000] ~ [-50db, 0db]
 */
static inline float volume_to_factor(int volume)
{
	return (volume > 0) ? expf((1000.0 - XMIN(volume, 1000)) * (-0.005756463)) : 0;
}

static inline int factor_to_volume(float factor)
{
	return (factor > 0) ? 1000 - XCLAMP((int)((-173.717792761) * logf(factor) + 0.5), 0, 1000) : 0;
}

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_AUDIO_H__ */
