# Audio Playback (audioplayback)

Audio playback (speaker output).

## Device Type

`DEVICE_TYPE_AUDIOPLAYBACK`

## Structure

```c
struct audio_playback_t {
    char * name;
    int (*start)(struct audio_playback_t * playback, int rate, int channel);
    int (*write)(struct audio_playback_t * playback, float * samples, int nsample);
    int (*stop)(struct audio_playback_t * playback);
    int (*ioctl)(struct audio_playback_t * playback, const char * cmd, void * arg);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_audio_playback(name)` | Find an audio playback device by name |
| `register_audio_playback(playback, drv)` | Register an audio playback device |
| `unregister_audio_playback(playback)` | Unregister an audio playback device |
| `audio_playback_start/stop(playback, ...)` | Start/stop playback |
| `audio_playback_write(playback, samples, nsample)` | Write audio samples |
| `audio_playback_get/set_volume(playback)` | Get/set volume |

## Description

Audio playback (speaker output) interface. Plays audio samples at a specified sample rate and channel count. Supports volume control and generic ioctl.
