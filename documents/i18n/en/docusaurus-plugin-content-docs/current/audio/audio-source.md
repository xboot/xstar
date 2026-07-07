# Audio Source (audiosource)

Audio input abstraction supporting multiple input sources.

## Structure

```c
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
```

## Creation Functions

| Function | Description |
|------|------|
| `audio_source_alloc()` | Allocate an empty Source |
| `audio_source_alloc_from_xfs(ctx, filename)` | Read from XFS file (WAV/QOA) |
| `audio_source_alloc_from_capture(name, rate, channel)` | Capture from microphone |
| `audio_source_alloc_from_memory(rate, channel, depth, mem, len)` | From in-memory PCM data |
| `audio_source_alloc_afsk(mark, space, rate, bit, packet, len)` | AFSK modem |
| `audio_source_alloc_tone(type, rate, channel, freq, ms)` | Waveform generator |
| `audio_source_alloc_noise(rate, channel)` | White noise generator |
| `audio_source_alloc_custom(rate, channel, cb, data)` | User-defined callback |
| `audio_source_alloc_from_mixer(mixer)` | Wrap a mixer as a Source |

## Control Functions

| Function | Description |
|------|------|
| `audio_source_read(s)` | Read an audio frame |
| `audio_source_seek(s, offset)` | Seek to a position |
| `audio_source_tell(s)` | Get current position |
| `audio_source_length(s)` | Get total length |
| `audio_source_get/set_volume(s, vol)` | Get/set volume |
| `audio_source_free(s)` | Free a Source |

## Tone Types

`audio_source_alloc_tone` supports the following waveforms:

| Type | Description |
|------|------|
| `"sine"` | Sine wave |
| `"square"` | Square wave |
| `"triangle"` | Triangle wave |
| `"sawtooth"` | Sawtooth wave |

## ioctl Commands {#ioctl}

Common ioctl commands (all sources should support):

| Command String | shash | arg Type | Description |
|---------------|-------|----------|-------------|
| `"audio-source-get-volume"` | — | `int *` | Get volume [0, 1000] |
| `"audio-source-set-volume"` | — | `int *` | Set volume [0, 1000] |

`audio_source_get_volume()` / `audio_source_set_volume()` are convenience wrappers around these ioctls. Each source can also define its own commands — see each source implementation for details.

## Data Flow Convention

`audio_source_read(s)` internally performs:

```c
return audio_filter_process(s->filter, s->read(s));
```

That is, the source's built-in `filter` is automatically applied to the read frame. Use `audio_source_filter_apply()` to replace or add filters. See [Audio Effect](./audio-effect) and [Usage Examples](./usage-examples) for more.
