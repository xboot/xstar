# Fast Fourier Transform (fft)

Fast Fourier Transform computing forward and inverse DFT on power-of-two sized data, using precomputed lookup tables and bit-reversal permutation.

## Data Structure

```c
struct fft_t {
    struct complex_t * interleave;
    struct complex_t * lut;
    unsigned int * bitinverse;
    unsigned int size;
};
```

## API

```c
struct fft_t * fft_new(int size_log2);
void fft_free(struct fft_t * fft);
void fft_process_forward_complex(struct fft_t * fft, struct complex_t * out, struct complex_t * in);
void fft_process_forward(struct fft_t * fft, struct complex_t * out, float * in);
void fft_process_inverse(struct fft_t * fft, float * out, struct complex_t * in);
```

- `fft_new` — Allocate FFT context for size 2^size_log2, precomputing lookup tables and bit-reversal permutation. Returns NULL on failure
- `fft_free` — Free FFT context and all precomputed resources
- `fft_process_forward_complex` — Forward DFT with complex input, producing complex output
- `fft_process_forward` — Forward DFT with real input, producing complex output
- `fft_process_inverse` — Inverse DFT with complex input, producing real output

## Example

```c
struct fft_t * fft = fft_new(10);
if(!fft)
    return;

float in[1024];
struct complex_t out[1024];

fft_process_forward(fft, out, in);

fft_free(fft);
```
