# CRC-8 Checksum (crc8)

CRC-8 cyclic redundancy check, using the **CRC-8/SMBUS** algorithm standard. Chainable by feeding the previous CRC output as the initial value.

## Algorithm Parameters

| Parameter | Value |
|-----------|-------|
| Name | CRC-8/SMBUS |
| Polynomial | `0x07` (x⁸ + x² + x + 1) |
| Init | `0x00` |
| RefIn | False |
| RefOut | False |
| XorOut | `0x00` |
| Check | `"123456789"` → `0xF4` |

## API

```c
uint8_t crc8_sum(uint8_t crc, const uint8_t * buf, int len);
```

- `crc` — Initial value (typically `0` for the first call)
- `buf` — Data buffer
- `len` — Data length

Returns the CRC-8 checksum.

## Example

### Single Checksum

```c
uint8_t msg[] = { 'x', 'b', 'o', 'o', 't' };
uint8_t checksum = crc8_sum(0, msg, sizeof(msg));
/* checksum = 0x01 */
```

### Chained Calls

```c
uint8_t part1[] = { 0x01, 0x02 };
uint8_t part2[] = { 0x03, 0x04 };
uint8_t crc = crc8_sum(0, part1, sizeof(part1));
crc = crc8_sum(crc, part2, sizeof(part2));
```
