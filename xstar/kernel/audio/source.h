#ifndef __XSTAR_KERNEL_AUDIO_SOURCE_H__
#define __XSTAR_KERNEL_AUDIO_SOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <kernel/xfs/xfs.h>
#include <kernel/audio/effect.h>

struct audio_source_t {
	struct audio_filter_t * filter;

	int (*seek)(struct audio_source_t * s, int offset);
	int (*tell)(struct audio_source_t * s);
	int (*length)(struct audio_source_t * s);
	struct audio_frame_t * (*read)(struct audio_source_t * s);
	int (*ioctl)(struct audio_source_t * s, const char * cmd, void * arg);
	void (*destroy)(struct audio_source_t * s);
	void * priv;
};

struct audio_source_t * audio_source_alloc(void);
struct audio_source_t * audio_source_alloc_from_xfs(struct xfs_context_t * ctx, const char * filename);
struct audio_source_t * audio_source_alloc_from_capture(const char * name, int rate, int channel);
struct audio_source_t * audio_source_alloc_from_memory(int rate, int channel, int depth, void * mem, int len);
struct audio_source_t * audio_source_alloc_afsk(int mark_hz, int space_hz, int sample_rate, int bit_rate, unsigned char * packet, int len);
struct audio_source_t * audio_source_alloc_tone(char * type, int rate, int channel, int frequency, int millisecond);
struct audio_source_t * audio_source_alloc_noise(int rate, int channel);
struct audio_source_t * audio_source_alloc_custom(int rate, int channel, int (*cb)(float * samples, int nsample, void * data), void * data);
void audio_source_free(struct audio_source_t * s);

struct audio_filter_t * audio_source_filter_apply(struct audio_source_t * s, const char * json, int len);
void audio_source_filter_bypass(struct audio_source_t * s, int id, int bypass);
void audio_source_filter_setup(struct audio_source_t * s, int id, const char * json, int len);
void audio_source_filter_bypass_by_name(struct audio_source_t * s, const char * name, int bypass);
void audio_source_filter_setup_by_name(struct audio_source_t * s, const char * name, const char * json, int len);
int audio_source_seek(struct audio_source_t * s, int offset);
int audio_source_tell(struct audio_source_t * s);
int audio_source_length(struct audio_source_t * s);
struct audio_frame_t * audio_source_read(struct audio_source_t * s);
int audio_source_ioctl(struct audio_source_t * s, const char * cmd, void * arg);
int audio_source_get_volume(struct audio_source_t * s);
void audio_source_set_volume(struct audio_source_t * s, int vol);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_AUDIO_SOURCE_H__ */
