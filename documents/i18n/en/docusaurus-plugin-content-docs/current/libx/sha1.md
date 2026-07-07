# SHA-1 Hash (sha1)

SHA-1 secure hash algorithm, 160-bit digest computation. Supports one-shot and streaming interfaces.

## Constants and Data Structure

```c
#define SHA1_DIGEST_SIZE (20)

struct sha1_ctx_t {
    uint64_t count;
    uint8_t buf[64];
    uint32_t state[5];
};
```

## API

### One-shot

```c
const uint8_t * sha1_hash(const void * data, int len, uint8_t * digest);
```

- `data` — Input data
- `len` — Data length
- `digest` — Output 20-byte digest

Returns `digest` pointer.

### Streaming

```c
void sha1_init(struct sha1_ctx_t * ctx);
void sha1_update(struct sha1_ctx_t * ctx, const void * data, int len);
const uint8_t * sha1_final(struct sha1_ctx_t * ctx);
```

- `sha1_init` — Initialize context
- `sha1_update` — Feed data, may be called multiple times
- `sha1_final` — Finalize and return digest pointer (points to internal context buffer)

## Example

### One-shot

```c
uint8_t msg[] = { 'x', 'b', 'o', 'o', 't' };
uint8_t digest[SHA1_DIGEST_SIZE];
sha1_hash(msg, sizeof(msg), digest);
/* digest = 0x7182df07 c7f20675 0fe2ed98 580ab526 cb839f5d */
```

### Streaming

```c
struct sha1_ctx_t ctx;
uint8_t digest[SHA1_DIGEST_SIZE];

sha1_init(&ctx);
sha1_update(&ctx, chunk1, len1);
sha1_update(&ctx, chunk2, len2);
sha1_final(&ctx);
```
