# QR Code Generator (qrcgen)

QR Code generator supporting versions 1-40, four error correction levels, and eight mask patterns.

## Data Structures

```c
#define QRCGEN_VERSION_MIN      (1)
#define QRCGEN_VERSION_MAX      (40)
#define QRCGEN_BUFFER_LEN(v)    ((((v) * 4 + 17) * ((v) * 4 + 17) + 7) / 8 + 1)
#define QRCGEN_BUFFER_LEN_MAX   QRCGEN_BUFFER_LEN(QRCGEN_VERSION_MAX)

enum qrcgen_ecc_t {
    QRCGEN_ECC_LOW,
    QRCGEN_ECC_MEDIUM,
    QRCGEN_ECC_QUARTILE,
    QRCGEN_ECC_HIGH,
};

enum qrcgen_mask_t {
    QRCGEN_MASK_AUTO  = -1,
    QRCGEN_MASK_0     = 0,
    QRCGEN_MASK_1     = 1,
    QRCGEN_MASK_2     = 2,
    QRCGEN_MASK_3     = 3,
    QRCGEN_MASK_4     = 4,
    QRCGEN_MASK_5     = 5,
    QRCGEN_MASK_6     = 6,
    QRCGEN_MASK_7     = 7,
};
```

## API

```c
int qrcgen_encode_text(const char * txt, uint8_t * tmp, uint8_t * qrc, enum qrcgen_ecc_t ecc, int minv, int maxv, enum qrcgen_mask_t mask, int boost);
int qrcgen_encode_binary(uint8_t * buf, int len, uint8_t * qrc, enum qrcgen_ecc_t ecc, int minv, int maxv, enum qrcgen_mask_t mask, int boost);
int qrcgen_get_size(uint8_t * qrc);
int qrcgen_get_pixel(uint8_t * qrc, int x, int y);
char * qrcgen_tostring(const char * txt, int invert);
```

- `qrcgen_encode_text` — Encode text string into QR code. `tmp` is workspace buffer (QRCGEN_BUFFER_LEN_MAX bytes), `qrc` is output buffer, `ecc` is error correction level, `minv`/`maxv` are version range, `mask` is mask pattern (QRCGEN_MASK_AUTO for auto), `boost` enables ECI boost. Returns 1 on success
- `qrcgen_encode_binary` — Encode binary data of `len` bytes into QR code. Parameters same as above except no `tmp` workspace. Returns 1 on success
- `qrcgen_get_size` — Return the side length (number of modules per side) of the generated QR code
- `qrcgen_get_pixel` — Return 1 if the pixel at (x, y) is dark (data module), 0 if light
- `qrcgen_tostring` — Convenience function that encodes `txt` and returns a string representation. `invert` swaps dark/light rendering. Returns malloc'd string on success, NULL on failure

## Example

```c
uint8_t tmp[QRCGEN_BUFFER_LEN_MAX];
uint8_t qrc[QRCGEN_BUFFER_LEN_MAX];

if(qrcgen_encode_text("Powered by xstar", tmp, qrc, QRCGEN_ECC_LOW, 1, 40, QRCGEN_MASK_AUTO, 1))
{
    int size = qrcgen_get_size(qrc);
    for(int y = 0; y < size; y++)
    {
        for(int x = 0; x < size; x++)
        {
            int pixel = qrcgen_get_pixel(qrc, x, y);
        }
    }
}

char * str = qrcgen_tostring("Powered by xstar", 0);
if(str)
{
    xos_mem_free(str);
}
```
