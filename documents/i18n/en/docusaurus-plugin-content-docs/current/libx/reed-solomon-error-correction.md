# Reed-Solomon Error Correction (rs)

Forward Error Correction (FEC) module based on the Reed-Solomon algorithm. Adds parity symbols to data, allowing the receiver to automatically detect and correct transmission errors. Suitable for data protection over unreliable channels.

## Principle

Reed-Solomon codes operate over the GF(2^8) finite field, with each symbol being 1 byte. During encoding, `nroots` parity symbols are generated for the data. During decoding, up to `nroots / 2` symbol errors can be corrected (including both data and parity symbols).

```
Encoding: [data len bytes] → [data len bytes] + [parity nroots bytes]
Decoding: [data len bytes] + [parity nroots bytes] → corrected [data len bytes] + [parity nroots bytes]
```

## Default Parameters

`rsctx_alloc` uses the following default parameters:

| Parameter | Value | Description |
|-----------|-------|-------------|
| Symbol size | 8 bit | 1 byte per symbol |
| Field polynomial | 0x11d | Primitive polynomial of GF(2^8) |
| First consecutive root | 0 | First root of the generator polynomial |
| Primitive element | 1 | Finite field primitive element |

The maximum block length is 255 bytes (2^8 - 1), and data length must satisfy `len <= 255 - nroots`.

## Data Structure

```c
struct rsctx_t {
    int mm;                     /* Bits per symbol */
    int nn;                     /* Symbols per block = (1 << mm) - 1 */
    unsigned char * alpha_to;   /* Antilog lookup table */
    unsigned char * index_of;   /* Log lookup table */
    unsigned char * genpoly;    /* Generator polynomial */
    int nroots;                 /* Number of parity symbols */
    int fcr;                    /* First consecutive root (exponent form) */
    int prim;                   /* Primitive element (exponent form) */
    int iprim;                  /* Inverse of prim */
};
```

## API

```c
struct rsctx_t * rsctx_alloc(int nroots);
```

Allocate an RS encoding/decoding context. `nroots` is the number of parity symbols, determining the error correction capability (can correct `nroots/2` symbol errors). Returns NULL on failure.

```c
void rsctx_free(struct rsctx_t * ctx);
```

Free the context and its internal lookup tables.

```c
int rsctx_encode(struct rsctx_t * ctx, unsigned char * data, unsigned int len, unsigned char * parity);
```

Encode, generating `nroots` parity symbols for `data` (length `len`) and writing them to `parity`. Requires `len <= 255 - nroots`. Returns 1 on success, 0 on invalid parameters.

```c
int rsctx_decode(struct rsctx_t * ctx, unsigned char * data, unsigned int len, unsigned char * parity);
```

Decode, performing error correction on `data` + `parity`. Both data and parity may be corrected (in-place modification). Returns 1 when no errors, 1 on successful correction, 0 on uncorrectable errors.

## Error Correction Capability

| nroots | Correctable symbols | Parity overhead |
|--------|--------------------|-----------------|
| 4 | 2 | 4 bytes |
| 8 | 4 | 8 bytes |
| 16 | 8 | 16 bytes |
| 32 | 16 | 32 bytes |

The larger nroots, the stronger the error correction capability, but the greater the parity overhead and the less available data length.

## Usage Examples

### Encoding

```c
int nroots = 16;
struct rsctx_t * ctx = rsctx_alloc(nroots);

unsigned char data[239] = { ... };   /* 255 - 16 = 239 bytes of data */
unsigned char parity[16];

rsctx_encode(ctx, data, 239, parity);

/* Send data + parity, 255 bytes total */
rsctx_free(ctx);
```

### Decoding and error correction

```c
int nroots = 16;
struct rsctx_t * ctx = rsctx_alloc(nroots);

unsigned char data[239];   /* Received data (may contain errors) */
unsigned char parity[16];  /* Received parity (may contain errors) */

if(rsctx_decode(ctx, data, 239, parity))
    /* Correction successful, data and parity have been restored */
else
    /* Too many errors, cannot correct */

rsctx_free(ctx);
```
