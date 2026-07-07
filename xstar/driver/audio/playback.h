#ifndef __XSTAR_DRIVER_AUDIO_PLAYBACK_H__
#define __XSTAR_DRIVER_AUDIO_PLAYBACK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/audio/audio.h>
#include <driver/device.h>
#include <driver/driver.h>

struct audio_playback_t {
	char * name;

	int (*start)(struct audio_playback_t * playback, int rate, int channel);
	int (*write)(struct audio_playback_t * playback, float * samples, int nsample);
	int (*stop)(struct audio_playback_t * playback);
	int (*ioctl)(struct audio_playback_t * playback, const char * cmd, void * arg);
	void * priv;
};

struct audio_playback_t * search_audio_playback(const char * name);
struct audio_playback_t * search_first_audio_playback(void);
struct device_t * register_audio_playback(struct audio_playback_t * playback, struct driver_t * drv);
void unregister_audio_playback(struct audio_playback_t * playback);

int audio_playback_start(struct audio_playback_t * playback, int rate, int channel);
int audio_playback_write(struct audio_playback_t * playback, float * samples, int nsample);
int audio_playback_stop(struct audio_playback_t * playback);
int audio_playback_ioctl(struct audio_playback_t * playback, const char * cmd, void * arg);
int audio_playback_get_volume(struct audio_playback_t * playback);
void audio_playback_set_volume(struct audio_playback_t * playback, int vol);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_AUDIO_PLAYBACK_H__ */
