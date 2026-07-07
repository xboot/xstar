# aplay

Audio playback command with configurable source, sink and effect chains.

## Usage

```
aplay -i=<source> -o=<sink> [-ie=<json>] [-oe=<json>] [-l]
```

## Options

| Option | Description |
|--------|-------------|
| `-i=<source>` | Input source (required) |
| `-o=<sink>` | Output sink (required) |
| `-ie=<json>` | Input-side effect chain JSON |
| `-oe=<json>` | Output-side effect chain JSON |
| `-l` | Loop playback, restart from beginning when source ends |

## Input Sources

| Type | Format | Description |
|------|--------|-------------|
| file | `file:<path>` | Play audio file from XFS (WAV/QOA) |
| capture | `capture:<device>,<rate>,<ch>` | Capture audio from microphone |
| tone | `tone:<waveform>,<rate>,<ch>,<freq>[,<ms>]` | Waveform generator |
| noise | `noise:<rate>,<ch>` | White noise generator |

- `waveform`: `sine`, `square`, `triangle`, `sawtooth`
- `ms`: Duration in milliseconds, omit or 0 for infinite loop
- `rate`: Sample rate (e.g. 48000)
- `ch`: Channel count (1=mono, 2=stereo)

## Output Sinks

| Type | Format | Description |
|------|--------|-------------|
| playback | `playback:<device>,<rate>,<ch>` | Play to hardware audio device |
| amplitude | `amplitude:<period_ms>` | Amplitude envelope analysis |
| spectrum | `spectrum:<period_ms>` | FFT spectrum analysis |
| vad | `vad:<start_hz>,<end_hz>` | Voice activity detection |
| afsk | `afsk:<mark_hz>,<space_hz>,<rate>,<bitrate>` | AFSK demodulation |

## Effect Chains

`-ie` and `-oe` accept a JSON object where keys are effect names and values are effect parameters. Multiple effects are processed in JSON key order.

Available effects:

| Effect | Parameters | Description |
|--------|------------|-------------|
| volume | `{"decibel": -6.0}` or `{"factor": 0.5}` | Volume adjustment |
| iir | `{"type": "lowpass", "frequency": 2000}` | IIR filter (lowpass/highpass/bandpass/lowshelf/highshelf/peaking/notch/allpass) |
| compressor | `{"threshold": -20, "ratio": 4}` | Dynamic range compression |
| limiter | `{"threshold": -1, "release": 50}` | Peak limiting |
| resample | `{"rate": 48000}` | Sample rate conversion |
| reshape | `{"channel": 2}` | Channel count conversion |
| mono | `{}` | Downmix to mono |
| panning | `{"weight": [1.0, 0.0, 0.0, 1.0]}` | Channel remapping |
| duplicate | `{}` | Buffer copy (isolates in-place mutations) |
| tremolo | `{"frequency": 5.0, "depth": 0.5}` | Tremolo effect |
| crystalizer | `{"intensity": 2.0}` | Sound enhancement |

## Examples

```bash
# Play audio file (loop)
aplay -i=file:music.wav -o=playback:playback-linux.0,48000,2 -l

# Sine wave 440Hz test tone
aplay -i=tone:sine,48000,2,440 -o=playback:playback-linux.0,48000,2

# White noise test
aplay -i=noise:48000,2 -o=playback:playback-linux.0,48000,2

# Microphone passthrough to speaker
aplay -i=capture:capture-linux.0,48000,2 -o=playback:playback-linux.0,48000,2

# Play file with input-side effect chain
aplay -i=file:music.wav -o=playback:playback-linux.0,48000,2 -l \
  -ie='{"volume":{"decibel":-6},"iir":{"type":"lowpass","frequency":2000}}'

# Play file with output-side limiter
aplay -i=file:music.wav -o=playback:playback-linux.0,48000,2 -l \
  -oe='{"limiter":{"threshold":-1}}'

# Microphone voice activity detection
aplay -i=capture:capture-linux.0,16000,1 -o=vad:300,800

# Microphone spectrum analysis
aplay -i=capture:capture-linux.0,48000,2 -o=spectrum:50
```
