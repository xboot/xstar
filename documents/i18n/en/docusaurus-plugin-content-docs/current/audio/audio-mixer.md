# Audio Mixer (audiomixer)

Mixes multiple audio sources into a single output.

## Structure

```c
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
```

## API

| Function | Description |
|------|------|
| `audio_mixer_alloc(rate, channel)` | Allocate a mixer |
| `audio_mixer_free(m)` | Free a mixer |
| `audio_mixer_add(m, source)` | Add an audio source |
| `audio_mixer_remove(m, source)` | Remove an audio source |
| `audio_mixer_clear(m)` | Clear all sources |
| `audio_mixer_read(m)` | Read the mixed result |
| `audio_mixer_get/set_volume(m, vol)` | Get/set volume |

## Notes

The mixer maintains an internal list of sources. `audio_mixer_read()` reads audio frames from all sources and mixes them. The mixer itself can also be wrapped as a Source via `audio_source_alloc_from_mixer(mixer)` for cascaded mixing.

## Automatic Format Normalization

When you call `audio_mixer_add()`, the mixer **automatically attaches a `resample+reshape` filter inside each source** to convert any sample-rate/channel input into the mixer's master format:

```c
char json[256];
xos_sprintf(json, "{\"resample\":{\"rate\":%d},\"reshape\":{\"channel\":%d}}",
            m->rate, m->channel);
audio_filter_alloc(json, length);
```

So you **don't need to manually align source and mixer formats** — a 16kHz mono microphone and a 48kHz stereo WAV can be mixed directly.

## Soft-Limiter Algorithm

The mixer uses dynamic range compression to avoid hard clipping. The core logic (see [`mixer.c:179-191`](https://github.com/anomalyco/xstar)):

```
For each output sample t = sample * mixed:
    if t > 1.0:
        mixed = 1.0 / t       ← immediate compression
        t = 1.0
    elif t < -1.0:
        mixed = -1.0 / t
        t = -1.0
    if mixed < 1.0:
        mixed += (1.0 - mixed) / 32.0    ← slow recovery (attack 1/32)
    output = t * factor       ← factor = user volume
```

Characteristics:
- **Instant response**: detect overflow and immediately reduce global `mixed`
- **Smooth recovery**: step back toward 1.0 at `1/32` per sample — no audible pumping
- **User volume independent**: `factor` is not involved in limiting, only scales the final output

## Cascaded Mixing Example

```c
struct audio_mixer_t * sub   = audio_mixer_alloc(48000, 2);
struct audio_mixer_t * main  = audio_mixer_alloc(48000, 2);

audio_mixer_add(sub, src_voice);
audio_mixer_add(sub, src_effect);

/* Wrap sub as a source to feed main */
struct audio_source_t * sub_as_src = audio_source_alloc_from_mixer(sub);
audio_mixer_add(main, sub_as_src);
audio_mixer_add(main, src_bgm);

/* Read final mix from main */
struct audio_frame_t * af = audio_mixer_read(main);
```

For a complete usage example, see [Usage Examples](./usage-examples) Scenario 3.
