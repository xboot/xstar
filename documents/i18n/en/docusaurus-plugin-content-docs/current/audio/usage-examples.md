# Usage Examples (usage-examples)

This document demonstrates common patterns of the XSTAR audio framework through 4 complete, runnable scenarios. Each example includes **Goal → Data Flow → Code → Key Notes**.

## Scenario 1: Play WAV File with Volume Adjustment

**Goal**: Read `music.wav` from XFS filesystem, apply 70% volume, and play through speaker.

**Data Flow**:
```
   music.wav (xfs)
        │
        ▼
   ┌──────────┐   filter:    ┌─────────────┐
   │ Source   │──{volume}──> │ Sink        │──> speaker
   │ file-wav │              │ playback    │
   └──────────┘              └─────────────┘
```

**Code**:
```c
#include <xstar.h>
#include <kernel/audio/source.h>
#include <kernel/audio/sink.h>

void play_music(struct xfs_context_t * ctx)
{
    struct audio_source_t * src;
    struct audio_sink_t   * snk;
    struct audio_frame_t  * af;

    /* 1. Create WAV source */
    src = audio_source_alloc_from_xfs(ctx, "/music.wav");
    if(!src)
        return;

    /* 2. Apply 70% volume filter */
    audio_source_filter_apply(src,
        "{\"volume\":{\"factor\":0.7}}", -1);

    /* 3. Create playback sink (48k stereo) */
    snk = audio_sink_alloc_from_playback(NULL, 48000, 2);
    if(!snk)
    {
        audio_source_free(src);
        return;
    }

    /* 4. Main loop: read-write */
    while((af = audio_source_read(src)) && audio_frame_is_valid(af))
        audio_sink_write(snk, af);

    /* 5. Cleanup */
    audio_sink_free(snk);
    audio_source_free(src);
}
```

**Key Notes**:
- `audio_source_alloc_from_xfs()` selects wav/qoa decoder by file extension
- JSON `len` of `-1` means measure with `\0` terminator
- `audio_sink_alloc_from_playback()` auto-attaches `resample+reshape` filter, so source format mismatch is auto-adapted
- Loop exits when `af == NULL` or `audio_frame_is_valid(af) == 0`

---

## Scenario 2: Microphone Recording with VAD

**Goal**: Capture audio from microphone, detect whether someone is speaking, trigger callbacks.

**Data Flow**:
```
   microphone
        │
        ▼
   ┌──────────┐              ┌─────────┐
   │ Source   │─────────────>│ Sink    │──> callback: speech start/end
   │ capture  │              │ vad     │
   └──────────┘              └─────────┘
```

**Code**:
```c
void voice_detect_loop(void)
{
    struct audio_source_t * src;
    struct audio_sink_t   * snk;
    struct audio_frame_t  * af;

    /* 1. Mic source: 16kHz mono */
    src = audio_source_alloc_from_capture(NULL, 16000, 1);

    /* 2. VAD sink: start=300ms (high energy 300ms -> speech start)
     *              end=800ms   (low energy 800ms -> speech end) */
    snk = audio_sink_alloc_vad(300, 800);

    while(running)
    {
        af = audio_source_read(src);
        if(audio_frame_is_valid(af))
            audio_sink_write(snk, af);
    }

    audio_sink_free(snk);
    audio_source_free(src);
}
```

**Key Notes**:
- `sink-vad` implements a full VAD algorithm (energy threshold + sliding window)
- Query current speech state via `audio_sink_ioctl(snk, "vad-get-state", ...)`
- For sample rate mismatch, attach a `resample` filter on the source

---

## Scenario 3: Multi-Source Mixing to Speaker

**Goal**: Mix background music + beep + microphone, output simultaneously.

**Data Flow**:
```
   bgm.wav  ──> Source A ──┐
                            │
   beep.wav ──> Source B ──┼──> Mixer (48k stereo) ──> Sink playback
                            │            │
   microphone ─> Source C ──┘     auto: resample+reshape
                                   per source
```

**Code**:
```c
void mix_three_sources(struct xfs_context_t * ctx)
{
    struct audio_mixer_t  * mixer;
    struct audio_source_t * src_a, * src_b, * src_c;
    struct audio_sink_t   * snk;
    struct audio_frame_t  * af;

    /* 1. Create 48k stereo mixer */
    mixer = audio_mixer_alloc(48000, 2);

    /* 2. Create 3 sources */
    src_a = audio_source_alloc_from_xfs(ctx, "/bgm.wav");
    src_b = audio_source_alloc_from_xfs(ctx, "/beep.wav");
    src_c = audio_source_alloc_from_capture(NULL, 16000, 1);

    /* 3. Add to mixer (mixer auto-adds resample+reshape per source) */
    audio_mixer_add(mixer, src_a);
    audio_mixer_add(mixer, src_b);
    audio_mixer_add(mixer, src_c);

    /* 4. Set mixer master volume to 70% */
    audio_mixer_set_volume(mixer, 700);

    /* 5. Create output sink */
    snk = audio_sink_alloc_from_playback(NULL, 48000, 2);

    /* 6. Main loop */
    while(running)
    {
        af = audio_mixer_read(mixer);
        if(audio_frame_is_valid(af))
            audio_sink_write(snk, af);
    }

    /* 7. Cleanup */
    audio_sink_free(snk);
    audio_mixer_free(mixer);        /* auto-clears internal list */
    audio_source_free(src_a);
    audio_source_free(src_b);
    audio_source_free(src_c);
}
```

**Key Notes**:
- `audio_mixer_add()` auto-wraps each source with `resample+reshape` filter — no manual format unification needed
- Mixer has built-in **soft-limiter algorithm** that smoothly attenuates on overflow
- Mixer can be wrapped as a source via `audio_source_alloc_from_mixer(mixer)` for multi-level mixing trees

---

## Scenario 4: Tone Generator with FFT Spectrum

**Goal**: Generate a 1kHz sine wave and compute its real-time spectrum for display.

**Data Flow**:
```
   ┌────────────┐    ┌────────────┐
   │ Source     │───>│ Sink       │──> periodic FFT spectrum array
   │ tone(sine) │    │ spectrum   │
   └────────────┘    └────────────┘
```

**Code**:
```c
void show_spectrum(void)
{
    struct audio_source_t * src;
    struct audio_sink_t   * snk;
    struct audio_frame_t  * af;

    /* 1. 1kHz sine wave, 48k mono, 5 seconds */
    src = audio_source_alloc_tone("sine", 48000, 1, 1000, 5000);

    /* 2. Spectrum sink, emit spectrum every 50ms */
    snk = audio_sink_alloc_spectrum(50);

    /* 3. Feed data */
    while((af = audio_source_read(src)) && audio_frame_is_valid(af))
    {
        audio_sink_write(snk, af);

        /* 4. Fetch latest spectrum via ioctl */
        float * fft_bins = NULL;
        int     nbins    = 0;
        struct {
            float ** bins;
            int    * n;
        } arg = { &fft_bins, &nbins };
        if(audio_sink_ioctl(snk, "spectrum-get-bins", &arg) >= 0)
        {
            /* render fft_bins[0..nbins-1] to screen ... */
        }
    }

    audio_sink_free(snk);
    audio_source_free(src);
}
```

**Key Notes**:
- `tone` supports 4 waveforms: `"sine"` / `"square"` / `"triangle"` / `"sawtooth"`
- Duration is in ms; pass 0 for looping
- `sink-spectrum` uses libx FFT; `period` determines output frequency (ms)
- See [Audio Sink ioctl commands](./audio-sink#ioctl) for command string specs

---

## Pattern Summary

| Scenario | Key Combination |
|----------|----------------|
| Single-source playback | `source-file` + (optional filter) + `sink-playback` |
| Recording / analysis | `source-capture` + `sink-vad` / `sink-spectrum` / `sink-amplitude` |
| Multi-source mixing | `source × N` + `mixer` + `sink-playback` |
| Signal synthesis | `source-tone` / `source-noise` / `source-custom` + `sink-*` |
| Data transmission | `source-afsk` ↔ `sink-afsk` (audio modem) |
| In-memory effect | `source-mem` + `filter` + `sink-playback` |

For more extension scenarios, see the [Extending Guide](./extending).
