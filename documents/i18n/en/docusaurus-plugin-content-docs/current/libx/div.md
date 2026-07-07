# Integer Division (div)

Software integer division routines for platforms lacking hardware divide instructions (e.g., some ARM cores).

## API

```c
uint32_t udiv32(uint32_t value, uint32_t divisor);
uint32_t umod32(uint32_t value, uint32_t divisor);
int32_t sdiv32(int32_t value, int32_t divisor);
int32_t smod32(int32_t value, int32_t divisor);
uint64_t udiv64(uint64_t value, uint64_t divisor);
uint64_t umod64(uint64_t value, uint64_t divisor);
int64_t sdiv64(int64_t value, int64_t divisor);
int64_t smod64(int64_t value, int64_t divisor);
```

- `udiv32/64` — Unsigned division
- `umod32/64` — Unsigned modulo
- `sdiv32/64` — Signed division
- `smod32/64` — Signed modulo

## Example

### 32-bit Unsigned Division

```c
uint32_t q = udiv32(100, 7);
uint32_t r = umod32(100, 7);
```

### 64-bit Signed Division

```c
int64_t q = sdiv64(-1000000LL, 37LL);
int64_t r = smod64(-1000000LL, 37LL);
```
