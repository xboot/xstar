#ifndef __XSTAR_KERNEL_AUDIO_EFFECT_H__
#define __XSTAR_KERNEL_AUDIO_EFFECT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/audio/audio.h>

struct audio_effect_t {
	struct list_head_t list;
	const char * name;
	void * (*create)(void);
	void (*setup)(void * ctx, struct json_value_t * v);
	void (*prepare)(void * ctx, struct audio_frame_t * input);
	struct audio_frame_t * (*process)(void * ctx, struct audio_frame_t * input);
	void (*destroy)(void * ctx);
};

struct audio_effect_node_t {
	const struct audio_effect_t * ae;
	void * ctx;
	int bypass;
};

struct audio_filter_t {
	struct audio_effect_node_t * nodes;
	int nlen;
};

int register_audio_effect(struct audio_effect_t * ae);
int unregister_audio_effect(struct audio_effect_t * ae);

struct audio_filter_t * audio_filter_alloc(const char * json, int len);
void audio_filter_free(struct audio_filter_t * filter);
void audio_filter_bypass(struct audio_filter_t * filter, int id, int bypass);
void audio_filter_setup(struct audio_filter_t * filter, int id, const char * json, int len);
void audio_filter_bypass_by_name(struct audio_filter_t * filter, const char * name, int bypass);
void audio_filter_setup_by_name(struct audio_filter_t * filter, const char * name, const char * json, int len);
struct audio_frame_t * audio_filter_process(struct audio_filter_t * filter, struct audio_frame_t * input);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_AUDIO_EFFECT_H__ */
