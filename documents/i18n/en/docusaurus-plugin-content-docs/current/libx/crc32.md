# CRC-32 Checksum (crc32)

CRC-32 cyclic redundancy check, using the **CRC-32 (ISO 3309)** algorithm standard. Chainable by feeding the previous CRC output as the initial value.

## Algorithm Parameters

| Parameter | Value |
|-----------|-------|
| Name | CRC-32 (ISO 3309 / ITU-T V.42) |
| Polynomial | `0x04C11DB7` |
| Init | `0xFFFFFFFF` |
| RefIn | True |
| RefOut | True |
| XorOut | `0xFFFFFFFF` |
| Check | `"123456789"` → `0xCBF43926` |

## API

```c
uint32_t crc32_sum(uint32_t crc, const uint8_t * buf, int len);
```

- `crc` — Initial value (typically `0` for the first call)
- `buf` — Data buffer
- `len` — Data length

Returns the CRC-32 checksum.

## Example

### Single Checksum

```c
uint8_t msg[] = { 'x', 'b', 'o', 'o', 't' };
uint32_t checksum = crc32_sum(0, msg, sizeof(msg));
/* checksum = 0x68292dcb */
```

### Chained Calls

```c
uint8_t part1[] = { 0x01, 0x02 };
uint8_t part2[] = { 0x03, 0x04 };
uint32_t crc = crc32_sum(0, part1, sizeof(part1));
crc = crc32_sum(crc, part2, sizeof(part2));
```
