# Audio Capture (audiocapture)

Audio capture (microphone input).

## Device Type

`DEVICE_TYPE_AUDIOCAPTURE`

## Structure

```c
struct audio_capture_t {
    char * name;
    int (*start)(struct audio_capture_t * capture, int rate, int channel);
    int (*read)(struct audio_capture_t * capture, float * samples, int nsample);
    int (*stop)(struct audio_capture_t * capture);
    int (*ioctl)(struct audio_capture_t * capture, const char * cmd, void * arg);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_audio_capture(name)` | Find an audio capture device by name |
| `register_audio_capture(capture, drv)` | Register an audio capture device |
| `unregister_audio_capture(capture)` | Unregister an audio capture device |
| `audio_capture_start/stop(capture, ...)` | Start/stop capture |
| `audio_capture_read(capture, samples, nsample)` | Read audio samples |
| `audio_capture_get/set_volume(capture)` | Get/set volume |

## Description

Audio capture (microphone input) interface. Captures audio samples at a specified sample rate and channel count, returning float samples. Supports volume control and generic ioctl.
