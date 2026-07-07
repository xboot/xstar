# Effect Plugins (effect-plugins)

The XSTAR audio framework ships with **11 built-in effect plugins**, all located in `xstar/kernel/audio/effect-*.c`. This document categorizes them as "format conversion → signal processing → special effects" with usage, JSON config, and key algorithms.

## Quick Reference

| Category | Name | Key JSON Params | Default | Stateful |
|----------|------|-----------------|---------|----------|
| Format | `resample` | `rate` (int) | 48000 | Yes (fraction accumulator) |
| Format | `reshape` | `channel` (int) | 1 | No |
| Format | `mono` | — | — | No |
| Format | `panning` | `weight` (array) | identity matrix | No |
| Utility | `duplicate` | — | — | No (memcpy only) |
| Signal | `volume` | `factor` or `decibel` (double) | 1.0 / 0dB | No |
| Signal | `compressor` | `threshold` (dBFS) / `ratio` / `attack` (ms) / `release` (ms) / `knee` (dB) / `makeup` (dB) | -20 / 4 / 5 / 100 / 6 / 0 | Yes (envelope) |
| Signal | `limiter` | `threshold` (dBFS) / `release` (ms) | -1.0 / 50 | Yes (instant gain) |
| Signal | `iir` | `type` / `frequency` / `quality` / `gain` | lowpass/1000/0.707/6.0 | Yes |
| Special | `tremolo` | `frequency` / `depth` (double) | 5.0 / 0.5 | Yes (phase) |
| Special | `crystalizer` | `intensity` (double) / `clip` (bool) | 2.0 / true | Yes (previous sample) |

---

## I. Format Conversion / Frame Handling

### 1. resample — Sample Rate Conversion

**Purpose**: Convert input sample rate to a target rate. Commonly used to unify various sources to the hardware-supported rate.

**JSON Config**:
```json
{
    "resample": {
        "rate": 48000
    }
}
```

**Algorithm**: Simple linear interval resampling (nearest-neighbor) using a `fraction` accumulator. For each input frame `fraction` increments by 1; when it exceeds 1, one frame is emitted and the step `step = input_rate / target_rate` is subtracted.

**Source**: `xstar/kernel/audio/effect-resample.c:104`

> 💡 This implementation prioritizes lightness and zero latency, with **no anti-aliasing filter**. For higher fidelity, chain an `iir` lowpass before `resample`.

---

### 2. reshape — Channel Count Transform

**Purpose**: Change the channel count. When reducing channels, takes the first N; when increasing, fills the remaining channels by **copying the last channel that was just written**.

**JSON Config**:
```json
{
    "reshape": {
        "channel": 2
    }
}
```

**Source**: `xstar/kernel/audio/effect-reshape.c`

---

### 3. mono — Downmix to Mono

**Purpose**: **Average** all channels to a single mono channel, preserving all channel information unlike `reshape`.

**JSON Config**:
```json
{
    "mono": {}
}
```

**Algorithm**: `out[n] = (in[n*ch + 0] + in[n*ch + 1] + ... + in[n*ch + ch-1]) / ch`

**Source**: `xstar/kernel/audio/effect-mono.c`

---

### 4. duplicate — Copy Audio Frame (Isolate In-Place Modifications)

**Purpose**: `memcpy` the input `audio_frame_t` sample data into the node's private buffer and return a new frame. **Format and sample values are identical** (rate/channel/frames all match the input).

**JSON Config**:
```json
{
    "duplicate": {}
}
```

**Why it's needed**: Different effects in the framework handle input frames differently:
- Some effects **modify input in-place** (`volume`, `tremolo`, `crystalizer`)
- Some effects **don't modify input**, writing to their own private buffer (`resample`, `reshape`, `mono`, and `duplicate` itself)

When upstream code needs to **preserve the original frame data** for other consumers, insert `duplicate` early in the chain so that subsequent in-place effects operate on a copy instead of the original.

**Example scenario**: The same source feeds both a speaker (with volume attenuation) and a spectrum analyzer (needs raw levels):
```
source ──> duplicate ──> volume ──> sink-playback
                 │
                 └─────> downstream using audio_filter_process for spectrum (unaffected by volume)
```

**Source**: `xstar/kernel/audio/effect-duplicate.c:72`

---

### 5. panning — Channel Matrix Remap

**Purpose**: Linear combination of channels via an N×N weight matrix. Useful for pan adjustment, stereo-to-mono, multi-channel downmix, etc.

**JSON Config** (2×2 stereo passthrough):
```json
{
    "panning": {
        "weight": [
            1.0, 0.0,
            0.0, 1.0
        ]
    }
}
```

Stereo to mono (half from each):
```json
{
    "panning": {
        "weight": [ 0.5, 0.5 ]
    }
}
```

Stereo left/right swap (L↔R):
```json
{
    "panning": {
        "weight": [
            0.0, 1.0,
            1.0, 0.0
        ]
    }
}
```

**Algorithm**: `out[v] = Σ weight[u][v] * in[u]`, up to 32 channels.

**Source**: `xstar/kernel/audio/effect-panning.c`

---

## II. Signal Processing

### 6. volume — Volume / Decibel Adjust

**Purpose**: Scale amplitude by factor or decibel value.

**JSON Config** (pick one):
```json
{
    "volume": { "factor": 0.5 }
}
```
```json
{
    "volume": { "decibel": -6.0 }
}
```

**Conversion**: `factor = 10^(decibel/20)`, so `-6dB ≈ 0.501`.

**Source**: `xstar/kernel/audio/effect-volume.c:89`

---

### 7. compressor — Dynamic Range Compressor

**Purpose**: When the input level exceeds the threshold, smoothly reduce gain by a configurable N:1 ratio, thereby **compressing the dynamic range**: louder parts get quieter, quieter parts become relatively louder (and the overall loudness can be boosted via `makeup`). Unlike `limiter` (a hard, instant ceiling), `compressor` provides tunable `ratio`, `attack`, `release`, `knee` and `makeup`, sounds more natural, and is the fundamental dynamics tool in playback / voice / recording chains.

**JSON configuration**:
```json
{
    "compressor": {
        "threshold": -20.0,
        "ratio": 4.0,
        "attack": 5.0,
        "release": 100.0,
        "knee": 6.0,
        "makeup": 0.0
    }
}
```

**Parameters**:

| Parameter | Meaning | Unit | Range | Default |
|---|---|:---:|:---:|:---:|
| `threshold` | Soft-knee center; compression starts above this level | dBFS | [-60.0, 0.0] | -20.0 |
| `ratio` | Compression ratio N:1 (N dB input increase yields only 1 dB output increase) | — | [1.0, 100.0] | 4.0 |
| `attack` | Attack time (how fast gain reduces when level rises) | ms | [0.1, 1000.0] | 5.0 |
| `release` | Release time (how fast gain recovers when level falls) | ms | [1.0, 5000.0] | 100.0 |
| `knee` | Soft-knee width (transition region around the threshold) | dB | [0.0, 24.0] | 6.0 |
| `makeup` | Make-up gain applied after compression | dB | [-24.0, 24.0] | 0.0 |

**Algorithm** (log-domain, feed-forward):
```
Per frame:
  1. Take the maximum absolute value of all channels: peak
  2. Convert to dB:  in_db = 20 * log10(peak)
  3. Compute gain reduction gr_db (with soft knee):
       over  = in_db - threshold
       slope = 1 - 1/ratio
       if over <= -knee/2 : gr_db = 0
       if over >=  knee/2 : gr_db = slope * over
       else (knee region) : gr_db = slope * (over + knee/2)^2 / (2 * knee)
  4. Envelope follower (asymmetric one-pole smoothing):
       coef = (gr_db > env_db) ? attack_coef : release_coef
       env_db += (gr_db - env_db) * coef
  5. Total gain:  gain = 10^((-env_db + makeup)/20)
  6. Multiply every channel by gain (shared gain preserves stereo image)
```

Attack / release coefficients (same form as `limiter`):
- `attack_coef  = 1 - exp(-1000 / (attack  * rate))`
- `release_coef = 1 - exp(-1000 / (release * rate))`

**Recommended `ratio` values**:

| ratio | Character | Typical use |
|:---:|---|---|
| 1.5 ~ 2.0 | Very subtle | Mastering glue |
| 2.0 ~ 4.0 | Natural | **Vocals, loudness control (recommended starting point)** |
| 4.0 ~ 8.0 | Noticeable | Drums, bass, anything needing stable level |
| 8.0 ~ 20.0 | Aggressive | Approaches limiting; hard control |
| > 20.0 | Limiter-like | Behaves close to a limiter |

**compressor vs limiter**:

| Aspect | compressor | limiter |
|---|---|---|
| Trigger | Smooth soft-knee transition | Instant hard trigger |
| Ratio | Adjustable (1:1 ~ 100:1) | Fixed ∞:1 |
| attack | Adjustable (ms-scale) | 0 (zero attack) |
| Purpose | Control dynamics, raise loudness | Safety net against clipping |
| Chain position | Early / middle of the signal chain | Very end of the signal chain |

> 💡 **Typical combo**: `compressor (ratio 3:1, threshold -20, makeup +6dB)` → … → `limiter (-1 dBFS)` — compress dynamics first, then let the limiter catch any peaks.

**Source**: `xstar/kernel/audio/effect-compressor.c`

---

### 8. limiter — Peak Limiter

**Purpose**: Instantly compress the signal when input peak exceeds the threshold, then smoothly recover back to unity gain over the release time. Zero attack time ensures no sample ever overshoots the ceiling. Useful for output protection and overall loudness control.

**JSON Config**:
```json
{
    "limiter": {
        "threshold": -1.0,
        "release": 50.0
    }
}
```

**Parameters**:
- `threshold`: ceiling in **dBFS** (decibels relative to Full Scale, i.e. the ±1.0 floating-point sample range), range [-60.0, 0.0], default -1.0 dBFS. `0 dBFS` corresponds to a linear amplitude of 1.0 (the float-sample full scale); `-1.0 dBFS` corresponds to about 0.891. Peaks above this value are immediately attenuated. Conversion formula: `ceiling = 10^(threshold/20)`.
- `release`: release time in ms, range [1.0, 5000.0], default 50 ms. Controls how quickly gain recovers toward 1.0 via a first-order low-pass after a limit event (about 63.2% recovered within `release` ms).

**dBFS to linear amplitude reference**:

| threshold (dBFS) | ceiling (linear) | Meaning |
|:---:|:---:|---|
| 0 | 1.000 | Full scale, equivalent to no limiting |
| -1 | 0.891 | **Default**, leaves 1 dB headroom |
| -3 | 0.708 | About half power |
| -6 | 0.501 | Amplitude halved |
| -20 | 0.100 | Amplitude 1/10 |
| -40 | 0.010 | Amplitude 1/100 |
| -60 | 0.001 | Amplitude 1/1000, near silence |

**Recommended `threshold` values**:

| Use case | Recommended threshold | Notes |
|---|:---:|---|
| Mastering / final output protection | -0.3 ~ -1.0 dBFS | Leaves headroom against inter-sample peak clipping after D/A conversion |
| General playback protection (default) | -1.0 dBFS | Balances loudness and safety; suitable for most scenarios |
| Noticeable loudness control | -3 ~ -6 dBFS | Audible dynamic compression effect |
| Aggressive limiting | < -6 dBFS | Noticeably alters timbre; use with care |

**Algorithm**:
```
Per frame:
  1. Find max absolute value across all channels (peak)
  2. If peak * gain > ceiling: target gain = ceiling / peak (instant compress)
  3. Otherwise: gain approaches 1.0 via a one-pole filter (smooth recovery)
  4. Scale all channels with gain (shared gain preserves stereo imaging)
```

Release coefficient: `coef = 1 - exp(-1000 / (release * rate))`, where `rate` is the current frame's sample rate.

**Source**: `xstar/kernel/audio/effect-limiter.c`

---

### 9. iir — IIR Biquad Filter (most powerful)

**Purpose**: Second-order IIR Biquad filter with 8 types. Useful for lowpass, highpass, bandpass, notch, equalizer, etc.

**JSON Config**:
```json
{
    "iir": {
        "type": "lowpass",
        "frequency": 1000.0,
        "quality": 0.707,
        "gain": 6.0
    }
}
```

**type values**:

| Type | Purpose |
|------|---------|
| `"lowpass"` | Low-pass (default) |
| `"highpass"` | High-pass |
| `"bandpass"` | Band-pass |
| `"lowshelf"` | Low shelf (EQ bass) |
| `"highshelf"` | High shelf (EQ treble) |
| `"peaking"` | Peaking (EQ mid) |
| `"notch"` | Notch (remove specific frequency) |
| `"allpass"` | All-pass (phase only) |

**Parameters**:
- `frequency`: cutoff / center frequency (Hz)
- `quality`: Q factor (0.707 for Butterworth response)
- `gain`: gain in dB (only for shelf / peaking types)

**Algorithm**: Standard Biquad difference equation `y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]`, with coefficients computed per Robert Bristow-Johnson formulas. Each channel maintains its own state `xn1/xn2/yn1/yn2`.

**Online designer**: [https://arachnoid.com/BiQuadDesigner](https://arachnoid.com/BiQuadDesigner)

**Source**: `xstar/kernel/audio/effect-iir.c:101`

---

## III. Special Effects

### 10. tremolo — Tremolo

**Purpose**: Periodically modulate amplitude to create a quivering effect (vintage guitar, retro tone, etc).

**JSON Config**:
```json
{
    "tremolo": {
        "frequency": 5.0,
        "depth": 0.5
    }
}
```

**Parameters**:
- `frequency`: modulation frequency (Hz), 0.1 ~ 20000, typical 4 ~ 8 Hz
- `depth`: modulation depth 0.0 ~ 1.0; 0 = no effect, 1 = full mute depth

**Algorithm**: Pre-generates a sine lookup table; multiplies each sample by `1 - depth + depth * sin(2π*f*t)`.

**Source**: `xstar/kernel/audio/effect-tremolo.c`

---

### 11. crystalizer — High-Frequency Enhancer

**Purpose**: Multiply sample-to-sample delta by a factor to boost high frequencies, making audio "crisper". Essentially a simple highpass emphasis.

**JSON Config**:
```json
{
    "crystalizer": {
        "intensity": 2.0,
        "clip": true
    }
}
```

**Parameters**:
- `intensity`: enhancement strength -10.0 ~ 10.0
- `clip`: hard-clip to `[-1, 1]` or not

**Algorithm**: `out[n] = in[n] + intensity * (in[n] - in[n-1])`

**Source**: `xstar/kernel/audio/effect-crystalizer.c`

---

## Chain Composition Example

Multiple effects can be chained, processed in JSON order:

```json
{
    "resample":    { "rate": 48000 },
    "reshape":     { "channel": 2 },
    "iir":         { "type": "lowpass", "frequency": 8000.0 },
    "compressor":  { "threshold": -20.0, "ratio": 3.0, "makeup": 6.0 },
    "volume":      { "decibel": -3.0 },
    "crystalizer": { "intensity": 1.5 },
    "limiter":     { "threshold": -1.0, "release": 50.0 }
}
```

> ⚠️ Order matters: typically **format normalization first** (resample/reshape), **then signal processing** (iir/compressor/volume), **then effects**, **and a `limiter` at the end as a safety net** against clipping.

> 📌 **Unknown effects**: Unregistered or mistyped effect names (e.g. `"limitter"`, `"echo"`) inside the chain are **silently skipped** by `audio_filter_alloc()` without error, and do not break parsing of the remaining effects. Recognized effects are still chained in their original JSON order. For example:
>
> ```json
> {
>     "volume":   { "decibel": -3.0 },
>     "limitter": { "threshold": -1.0 },   // typo, ignored
>     "echo":     { "delay": 100 },        // not implemented, ignored
>     "limiter":  { "threshold": -1.0 }
> }
> ```
>
> The effective chain is `volume → limiter`, with 2 nodes. If **none** of the names in the object can be recognized, `audio_filter_alloc()` returns `NULL`. This behavior means typos will not crash the system, but they are also reported nowhere — please cross-check effect names against the [quick reference table](#quick-reference) during development.

## How to Choose an Effect

| Your need | Recommended effect |
|-----------|-------------------|
| Align sample rate/channels for mixing | `resample` + `reshape` |
| Master volume control | `volume` |
| Stereo to mono | `mono` or `panning` |
| Mono to stereo | `reshape` (channel=2) or `panning` |
| Isolate input from subsequent in-place effects | `duplicate` |
| Bass/treble enhancement | `iir` (lowshelf/highshelf) |
| Remove specific frequency noise | `iir` (notch) |
| Vintage tremolo effect | `tremolo` |
| Improve audio clarity | `crystalizer` |
| Control dynamic range / stabilize loudness / raise perceived loudness | `compressor` |
| Prevent output clipping / loudness control | `limiter` |

## Registration Mechanism

All effects auto-register via `core_initcall()`, following this template (see [Extending Guide](./extending)):

```c
static struct audio_effect_t my_effect = {
    .name    = "myeffect",
    .create  = my_create,
    .setup   = my_setup,
    .prepare = my_prepare,
    .process = my_process,
    .destroy = my_destroy,
};
static void my_effect_init(void) { register_audio_effect(&my_effect); }
core_initcall(my_effect_init);
```
