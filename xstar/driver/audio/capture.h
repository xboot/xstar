#ifndef __XSTAR_DRIVER_AUDIO_CAPTURE_H__
#define __XSTAR_DRIVER_AUDIO_CAPTURE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/audio/audio.h>
#include <driver/device.h>
#include <driver/driver.h>

struct audio_capture_t {
	char * name;

	int (*start)(struct audio_capture_t * capture, int rate, int channel);
	int (*read)(struct audio_capture_t * capture, float * samples, int nsample);
	int (*stop)(struct audio_capture_t * capture);
	int (*ioctl)(struct audio_capture_t * capture, const char * cmd, void * arg);
	void * priv;
};

struct audio_capture_t * search_audio_capture(const char * name);
struct audio_capture_t * search_first_audio_capture(void);
struct device_t * register_audio_capture(struct audio_capture_t * capture, struct driver_t * drv);
void unregister_audio_capture(struct audio_capture_t * capture);

int audio_capture_start(struct audio_capture_t * capture, int rate, int channel);
int audio_capture_read(struct audio_capture_t * capture, float * samples, int nsample);
int audio_capture_stop(struct audio_capture_t * capture);
int audio_capture_ioctl(struct audio_capture_t * capture, const char * cmd, void * arg);
int audio_capture_get_volume(struct audio_capture_t * capture);
void audio_capture_set_volume(struct audio_capture_t * capture, int vol);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_AUDIO_CAPTURE_H__ */
