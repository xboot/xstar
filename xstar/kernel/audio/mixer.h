#ifndef __XSTAR_KERNEL_AUDIO_MIXER_H__
#define __XSTAR_KERNEL_AUDIO_MIXER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/audio/source.h>

struct audio_mixer_t {
	struct list_head_t list;
	struct mutex_t lock;
	int rate;
	int channel;
	float mixed;
	float factor;
	struct audio_frame_t output;
	float * samples;
	int nsample;
};

struct audio_mixer_t * audio_mixer_alloc(int rate, int channel);
void audio_mixer_free(struct audio_mixer_t * m);
void audio_mixer_add(struct audio_mixer_t * m, struct audio_source_t * s);
void audio_mixer_remove(struct audio_mixer_t * m, struct audio_source_t * s);
void audio_mixer_clear(struct audio_mixer_t * m);
struct audio_frame_t * audio_mixer_read(struct audio_mixer_t * m);
int audio_mixer_get_volume(struct audio_mixer_t * m);
void audio_mixer_set_volume(struct audio_mixer_t * m, int vol);

struct audio_source_t * audio_source_alloc_from_mixer(struct audio_mixer_t * mixer);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_AUDIO_MIXER_H__ */
