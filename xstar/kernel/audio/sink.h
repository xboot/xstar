#ifndef __XSTAR_KERNEL_AUDIO_SINK_H__
#define __XSTAR_KERNEL_AUDIO_SINK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <kernel/audio/effect.h>

struct audio_sink_t {
	struct audio_filter_t * filter;

	void (*write)(struct audio_sink_t * s, struct audio_frame_t * af);
	int (*ioctl)(struct audio_sink_t * s, const char * cmd, void * arg);
	void (*destroy)(struct audio_sink_t * s);
	void * priv;
};

struct audio_sink_t * audio_sink_alloc(void);
struct audio_sink_t * audio_sink_alloc_from_playback(const char * name, int rate, int channel);
struct audio_sink_t * audio_sink_alloc_amplitude(int period);
struct audio_sink_t * audio_sink_alloc_spectrum(int period);
struct audio_sink_t * audio_sink_alloc_vad(int start, int end);
struct audio_sink_t * audio_sink_alloc_afsk(int mark_hz, int space_hz, int sample_rate, int bit_rate);
void audio_sink_free(struct audio_sink_t * s);

struct audio_filter_t * audio_sink_filter_apply(struct audio_sink_t * s, const char * json, int len);
void audio_sink_filter_bypass(struct audio_sink_t * s, int id, int bypass);
void audio_sink_filter_setup(struct audio_sink_t * s, int id, const char * json, int len);
void audio_sink_filter_bypass_by_name(struct audio_sink_t * s, const char * name, int bypass);
void audio_sink_filter_setup_by_name(struct audio_sink_t * s, const char * name, const char * json, int len);
void audio_sink_write(struct audio_sink_t * s, struct audio_frame_t * af);
int audio_sink_ioctl(struct audio_sink_t * s, const char * cmd, void * arg);
int audio_sink_get_volume(struct audio_sink_t * s);
void audio_sink_set_volume(struct audio_sink_t * s, int vol);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_AUDIO_SINK_H__ */
