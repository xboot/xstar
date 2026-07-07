# Audio Framework Overview (overview)

The XSTAR audio framework is a **lightweight, modular, JSON-driven** embedded audio processing subsystem located at `xstar/kernel/audio/`. Built around the classic **Source → Filter → Sink** pipeline model with a pluggable effect engine and mixer, it supports a wide range of audio applications including file playback, microphone capture, mixing, spectrum analysis, and AFSK modulation/demodulation.

## Framework Positioning

| Aspect | Description |
|--------|-------------|
| Data Format | Internally uses `float` interleaved samples (`audio_frame_t`) |
| Configuration | Effect chains built from JSON strings |
| Extension | Plugin registration via `register_audio_effect()`, auto-loaded via initcall |
| Abstraction | `kernel/audio` handles processing/orchestration; `driver/audio` handles raw PCM I/O |
| Code Size | ~6400 lines (11 effects + 11 sources + 6 sinks + 1 mixer) |

## Architecture Overview

```
                ┌─────────────────────────────────────────────┐
                │              kernel/audio                    │
                │                                              │
   ┌────────┐   │   ┌────────┐   ┌─────────┐   ┌────────┐    │   ┌──────────┐
   │ files  │──>│──>│ Source │──>│ Filter  │──>│  Sink  │──>│──>│ playback │
   │ mic    │   │   │ (read) │   │  Chain  │   │ (write)│    │   │  device  │
   │ memory │   │   └────────┘   │         │   └────────┘    │   └──────────┘
   │ tone   │   │        ▲       │ effect1 │        │        │
   │ ...    │   │        │       │ effect2 │        ▼        │
   └────────┘   │   ┌────┴────┐  │  ...    │   ┌─────────┐   │
                │   │  Mixer  │  └─────────┘   │ analyze │   │
                │   │ (mix N) │                │ vad/fft │   │
                │   └─────────┘                └─────────┘   │
                │                                              │
                └─────────────────────────────────────────────┘
                                    │
                                    ▼
                            audio_frame_t  (float interleaved samples)
```

**Core idea**: Everything is a `source` or `sink` with an attached `filter`. Multiple `sources` can be merged through a `mixer`, which can then be wrapped as a new `source` and fed back into the pipeline.

## Layered Model

| Layer | Files | Responsibility |
|-------|-------|----------------|
| 1. Data Carrier | `driver/audio/audio.h` | Defines `audio_frame_t`, provides volume/decibel conversion |
| 2. Effect Engine | `effect.h` / `effect.c` | Effect plugin registration, JSON parsing, filter chain management |
| 3. Source / Sink | `source.h` / `sink.h` | Unified I/O abstraction with built-in filter |
| 4. Mixer | `mixer.h` / `mixer.c` | Multi-source aggregation, auto-normalization, nestable as source |

## Core Data Flow

All components exchange data through `struct audio_frame_t`:

```c
struct audio_frame_t {
    int     rate;     /* Sample rate (Hz), e.g. 48000 */
    int     channel;  /* Channel count: 1=mono, 2=stereo */
    int     frames;   /* Frame count, typically ~5ms of data */
    float * samples;  /* Interleaved samples [L,R,L,R,...] */
};
```

**Typical read flow**:

```
audio_source_read(s)
        │
        ├──> s->read(s)                        ← Source-specific raw frame producer
        │
        └──> audio_filter_process(s->filter)   ← Auto-apply attached effect chain
                │
                └──> effect[0].process()
                     effect[1].process()
                     ...
                     return final frame
```

`audio_sink_write()` works in reverse: input frames pass through the sink's built-in filter before being written.

## Six Design Highlights

### 1. Unified Internal Float Format
All sources output `float` interleaved samples; all sinks convert to target format only at the hardware boundary. This lets every effect algorithm work with a single data type, drastically simplifying implementation.

### 2. JSON-Driven + shash Hashing
Effect chains are built from JSON, for example:

```json
{
    "resample": { "rate": 48000 },
    "reshape":  { "channel": 2 },
    "volume":   { "factor": 0.7 }
}
```

JSON field names are matched via `shash()` hashes inside `switch` statements, eliminating runtime string comparison while preserving readability. See [`effect-resample.c:70`](https://github.com/anomalyco/xstar).

### 3. Plugin Registration + initcall
Each effect self-registers into a global list via `register_audio_effect()`. Combined with multi-level `core_initcall()` / `pure_initcall()`, no manual dependency ordering is needed:

```c
static struct audio_effect_t volume = {
    .name = "volume", .create = ..., .setup = ..., ...
};
static void effect_volume_init(void)
{
    register_audio_effect(&volume);
}
core_initcall(effect_volume_init);
```

### 4. Filter Reuse
The same `audio_filter_t` is reused in three places:
- `source` built-in filter — input-side preprocessing
- `sink` built-in filter — output-side postprocessing
- `mixer` attaches `resample+reshape` per source to match the master format

### 5. Pipeline Nesting
`audio_source_alloc_from_mixer()` wraps an entire mixer's output as a new `source`, enabling multi-level mixing trees:

```
[music]  ─┐
[voice]  ─┼─> Mixer A ──┐
[effect] ─┘             ├─> Mixer B ──> Sink
              [notify] ──┘
```

### 6. Soft-Limiter Auto-Normalization
After mixing, if samples exceed `[-1, 1]`, the mixer adjusts a global attenuation factor `mixed` and recovers gradually at `1/32` per step — providing smooth dynamic range compression without hard clipping. See [`mixer.c:179-191`](https://github.com/anomalyco/xstar).

## Component Catalog

### Source (11 types)

| Name | File | Purpose |
|------|------|---------|
| `file-wav` | `source-file-wav.c` | WAV file decoder |
| `file-qoa` | `source-file-qoa.c` | QOA file decoder |
| `capture` | `source-capture.c` | Microphone capture |
| `memory` | `source-mem.c` | In-memory PCM data |
| `mixer` | `source-mixer.c` | Wrap a mixer as a source |
| `tone` | `source-tone.c` | Sine/square/triangle/sawtooth generator |
| `noise` | `source-noise.c` | White noise generator |
| `afsk` | `source-afsk.c` | AFSK modulation (data→audio) |
| `custom` | `source-custom.c` | User callback function |

### Sink (6 types)

| Name | File | Purpose |
|------|------|---------|
| `playback` | `sink-playback.c` | Output to hardware speaker |
| `amplitude` | `sink-amplitude.c` | Amplitude envelope detection |
| `spectrum` | `sink-spectrum.c` | FFT spectrum analysis |
| `vad` | `sink-vad.c` | Voice Activity Detection |
| `afsk` | `sink-afsk.c` | AFSK demodulation (audio→data) |

### Effect (11 built-in)

| Name | Purpose |
|------|---------|
| `volume` | Volume / decibel adjustment |
| `compressor` | Dynamic range compressor (soft-knee, adjustable ratio/attack/release/makeup) |
| `limiter` | Peak limiter (output protection) |
| `panning` | Multi-channel matrix mixing |
| `mono` | Downmix all channels to mono |
| `duplicate` | Copy audio frame to a private buffer (isolate downstream in-place modifications) |
| `reshape` | Change channel count |
| `resample` | Change sample rate |
| `iir` | IIR Biquad filter (8 types: lowpass/highpass/bandpass/notch/...) |
| `tremolo` | Tremolo (amplitude modulation) |
| `crystalizer` | High-frequency detail enhancer |

See [Effect Plugins](./effect-plugins) for configuration parameters and algorithms.

## Source Code Reference

| Structure / API | Location |
|-----------------|----------|
| `audio_frame_t` | `xstar/driver/audio/audio.h:10` |
| `audio_effect_t` | `xstar/kernel/audio/effect.h:11` |
| `audio_filter_t` | `xstar/kernel/audio/effect.h:27` |
| `audio_source_t` | `xstar/kernel/audio/source.h:12` |
| `audio_sink_t` | `xstar/kernel/audio/sink.h:11` |
| `audio_mixer_t` | `xstar/kernel/audio/mixer.h:10` |
| `audio_filter_process()` | `xstar/kernel/audio/effect.c:219` |
| `audio_mixer_read()` soft-limiter | `xstar/kernel/audio/mixer.c:159` |

## Relationship with Driver Layer

```
┌─────────────────────────────────────────┐
│        kernel/audio (this framework)     │
│  source / sink / effect / mixer          │
│  ── processing / orchestration / fmt ──  │
└────────────┬────────────┬───────────────┘
             │            │
             ▼            ▼
       audio_playback  audio_capture
       (driver/audio)  (driver/audio)
             │            │
             ▼            ▼
        hardware DAC  hardware ADC
```

The driver layer only provides raw PCM I/O (`audio_playback_write()` / `audio_capture_read()`); all mixing, effects, decoding, and format conversion happen in the kernel layer for portability.

## Reading Guide

| Your goal | Recommended reading |
|-----------|---------------------|
| Quickly build a playback program | [Usage Examples](./usage-examples) → Scenario 1 |
| Understand effect-chain JSON | [Audio Effect](./audio-effect) → [Effect Plugins](./effect-plugins) |
| Multi-source mixing | [Audio Mixer](./audio-mixer) → [Usage Examples](./usage-examples) Scenario 3 |
| Extend framework with custom formats/algorithms | [Extending Guide](./extending) |
| Component API reference | [Audio Frame](./audio-frame) / [Audio Source](./audio-source) / [Audio Sink](./audio-sink) |
