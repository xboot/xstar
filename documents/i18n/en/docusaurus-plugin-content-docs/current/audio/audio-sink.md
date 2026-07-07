# Audio Sink (audiosink)

Audio output abstraction supporting multiple output destinations.

## Structure

```c
struct audio_sink_t {
    struct audio_filter_t * filter;
    void (*write)(struct audio_sink_t * s, struct audio_frame_t * af);
    int (*ioctl)(struct audio_sink_t * s, const char * cmd, void * arg);
    void (*destroy)(struct audio_sink_t * s);
    void * priv;
};
```

## Creation Functions

| Function | Description |
|------|------|
| `audio_sink_alloc()` | Allocate an empty Sink |
| `audio_sink_alloc_from_playback(name, rate, channel)` | Hardware playback output |
| `audio_sink_alloc_amplitude(period)` | Amplitude envelope extraction |
| `audio_sink_alloc_spectrum(period)` | FFT spectrum analysis |
| `audio_sink_alloc_vad(start, end)` | Voice activity detection |
| `audio_sink_alloc_afsk(mark, space, rate, bit)` | AFSK modulation output |

## Control Functions

| Function | Description |
|------|------|
| `audio_sink_write(s, af)` | Write an audio frame |
| `audio_sink_ioctl(s, cmd, arg)` | Device control |
| `audio_sink_get/set_volume(s, vol)` | Get/set volume |
| `audio_sink_free(s)` | Free a Sink |

## ioctl Commands {#ioctl}

Common ioctl commands (all sinks should support):

| Command String | shash | arg Type | Description |
|---------------|-------|----------|-------------|
| `"audio-sink-get-volume"` | `0xe04cfa2b` | `int *` | Get volume [0, 1000] |
| `"audio-sink-set-volume"` | `0x10cbc7b7` | `int *` | Set volume [0, 1000] |

`audio_sink_get_volume()` / `audio_sink_set_volume()` are convenience wrappers around these ioctls.

Sink-specific extended command examples:

| sink type | Command String | Description |
|----------|---------------|-------------|
| `spectrum` | `"spectrum-get-bins"` | Get latest FFT spectrum array |
| `amplitude` | `"amplitude-get-value"` | Get current amplitude envelope |
| `vad` | `"vad-get-state"` | Get voice activity detection status |

See each sink's source for parameter details and return formats.

## Data Flow Convention

`audio_sink_write(s, af)` internally performs:

```c
s->write(s, audio_filter_process(s->filter, af));
```

That is, each frame passes through the sink's built-in `filter` before being written. For example, `sink-playback` has a built-in `resample+reshape` filter for automatic format adaptation. See [Audio Effect](./audio-effect) and [Usage Examples](./usage-examples) for more.
