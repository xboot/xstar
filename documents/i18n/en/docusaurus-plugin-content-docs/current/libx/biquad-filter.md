# Biquad Filter (biquad)

Biquadratic IIR filter, supporting 8 common filter types, used for frequency response shaping in audio and signal processing.

## Principle

The biquad filter is a standard implementation of a second-order IIR filter, with the transfer function:

```
H(z) = (b0 + b1·z⁻¹ + b2·z⁻²) / (1 + a1·z⁻¹ + a2·z⁻²)
```

The corresponding difference equation:

```
y[n] = b0·x[n] + b1·x[n-1] + b2·x[n-2] - a1·y[n-1] - a2·y[n-2]
```

Coefficients are calculated from the sample rate, cutoff frequency, quality factor Q, and gain based on the filter type, referencing [BiQuadDesigner](https://arachnoid.com/BiQuadDesigner).

## Filter Types

| Enum Value | Type | Description |
|--------|------|------|
| `BIQUAD_FILTER_TYPE_LOWPASS` | Low-pass | Attenuates high frequencies, preserves low frequencies |
| `BIQUAD_FILTER_TYPE_HIGHPASS` | High-pass | Attenuates low frequencies, preserves high frequencies |
| `BIQUAD_FILTER_TYPE_BANDPASS` | Band-pass | Preserves only the frequency band around the center frequency |
| `BIQUAD_FILTER_TYPE_LOWSHELF` | Low shelf | Boosts or cuts low frequencies |
| `BIQUAD_FILTER_TYPE_HIGHSHELF` | High shelf | Boosts or cuts high frequencies |
| `BIQUAD_FILTER_TYPE_PEAKING` | Peaking | Boosts or cuts a specified frequency |
| `BIQUAD_FILTER_TYPE_NOTCH` | Notch | Attenuates a specified frequency (narrow band) |
| `BIQUAD_FILTER_TYPE_ALLPASS` | All-pass | Changes phase only, does not alter magnitude |

## Data Structure

```c
struct biquad_filter_t {
    float b0, b1, b2;   /* Numerator coefficients */
    float a1, a2;       /* Denominator coefficients (a0 normalized to 1) */
    float xn1, xn2;     /* Input history: x[n-1], x[n-2] */
    float yn1, yn2;     /* Output history: y[n-1], y[n-2] */
};
```

## API

```c
struct biquad_filter_t * biquad_alloc(enum biquad_filter_type_t type, float samplerate, float frequency, float quality, float gain);
```

Allocate and initialize a filter. `samplerate` is the sample rate (Hz), `frequency` is the cutoff/center frequency (Hz), `quality` is the quality factor Q, `gain` is the gain (dB, valid only for shelf and peaking types). Returns `NULL` on failure.

```c
void biquad_free(struct biquad_filter_t * filter);
```

Free the filter.

```c
void biquad_init(struct biquad_filter_t * filter, enum biquad_filter_type_t type, float samplerate, float frequency, float quality, float gain);
```

Initialize the coefficients of an existing filter structure without allocating memory. After calling, `biquad_clear` must be called manually to clear the history state.

```c
void biquad_clear(struct biquad_filter_t * filter);
```

Clear the filter's historical input/output state (zero xn1, xn2, yn1, yn2), does not change coefficients.

```c
void biquad_process(struct biquad_filter_t * filter, float * input, float * output, int len);
```

Filter `len` samples from the `input` buffer and write the result to `output`. Supports block processing; history state is automatically maintained across calls.

## Usage Example

```c
struct biquad_filter_t * f = biquad_alloc(BIQUAD_FILTER_TYPE_LOWPASS, 44100.0f, 1000.0f, 0.707f, 0.0f);

float in[256], out[256];
while(read_audio(in, 256))
{
    biquad_process(f, in, out, 256);
    write_audio(out, 256);
}

biquad_free(f);
```
