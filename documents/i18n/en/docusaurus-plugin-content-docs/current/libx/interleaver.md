# Interleaver (interleaver)

Data interleaving and deinterleaving module based on pseudo-random permutation. By reordering byte order, consecutive burst errors are scattered into random errors, improving anti-burst interference capability when used with error correction codes (e.g., Reed-Solomon).

## Principle

Interleaving rearranges data in a pseudo-random order, and deinterleaving restores the original order. Example:

```
Original data:   [A B C D E F G H]
Interleaved:     [D A H E B G C F]    ← rearranged according to pattern
Burst error during transmission:              ↑↑↑
Received data:  [D A X X B G C F]    ← 2 consecutive bytes corrupted
Deinterleaved:  [A B C X E X G H]    ← errors are scattered, no longer consecutive
```

The same `size` and `seed` generate the same interleaving pattern. Both communicating parties only need to agree on these two parameters.

## Data Structure

```c
struct interleaver_t {
    int size;           /* Data block length (bytes) */
    int * pattern;      /* Interleaving pattern: pattern[i] indicates output byte i comes from input byte pattern[i] */
    int * ipattern;     /* Deinterleaving pattern: ipattern[i] indicates deinterleaved output byte i comes from input byte ipattern[i] */
};
```

## API

```c
struct interleaver_t * interleaver_alloc(int size, int seed);
```

Allocate an interleaver. `size` is the data block length, `seed` is the pseudo-random generator seed. Internally uses LFSR pseudo-random numbers with Fisher-Yates shuffle to generate the interleaving pattern, and simultaneously computes the inverse pattern for deinterleaving. Returns NULL on failure.

```c
void interleaver_free(struct interleaver_t * ctx);
```

Free the interleaver.

```c
void interleave(struct interleaver_t * ctx, uint8_t * input, uint8_t * output);
```

Interleave, rearranging `input` according to `pattern` and writing to `output`. Both `input` and `output` are `size` bytes long. They may point to the same buffer but the result will be incorrect (separate buffers required).

```c
void deinterleave(struct interleaver_t * ctx, uint8_t * input, uint8_t * output);
```

Deinterleave, restoring `input` to original order according to `ipattern` and writing to `output`. Same as above, `input` and `output` must be different buffers.

## Usage Examples

### Used with Reed-Solomon

```c
int nroots = 16;
int data_len = 239;
int total = data_len + nroots;

struct rsctx_t * rs = rsctx_alloc(nroots);
struct interleaver_t * il = interleaver_alloc(total, 12345);

unsigned char data[239] = { ... };
unsigned char parity[16];
unsigned char encoded[255];
unsigned char interleaved[255];

rsctx_encode(rs, data, data_len, parity);

memcpy(encoded, data, data_len);
memcpy(encoded + data_len, parity, nroots);
interleave(il, encoded, interleaved);

/* Send interleaved ... */

/* Receiver side */
unsigned char received[255];
unsigned char deinterleaved[255];

deinterleave(il, received, deinterleaved);
memcpy(data, deinterleaved, data_len);
memcpy(parity, deinterleaved + data_len, nroots);
rsctx_decode(rs, data, data_len, parity);

interleaver_free(il);
rsctx_free(rs);
```

### Interleave/deinterleave only

```c
struct interleaver_t * il = interleaver_alloc(64, 42);

uint8_t original[64] = { ... };
uint8_t shuffled[64];
uint8_t restored[64];

interleave(il, original, shuffled);
deinterleave(il, shuffled, restored);

/* restored is identical to original */

interleaver_free(il);
```
