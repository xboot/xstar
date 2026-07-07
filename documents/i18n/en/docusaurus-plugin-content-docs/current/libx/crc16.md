# CRC-16 Checksum (crc16)

CRC-16 cyclic redundancy check, using the **CRC-16/XMODEM** algorithm standard. Chainable by feeding the previous CRC output as the initial value.

## Algorithm Parameters

| Parameter | Value |
|-----------|-------|
| Name | CRC-16/XMODEM |
| Polynomial | `0x1021` (x¹⁶ + x¹² + x⁵ + 1) |
| Init | `0x0000` |
| RefIn | False |
| RefOut | False |
| XorOut | `0x0000` |
| Check | `"123456789"` → `0x31C3` |

## API

```c
uint16_t crc16_sum(uint16_t crc, const uint8_t * buf, int len);
```

- `crc` — Initial value (typically `0` for the first call)
- `buf` — Data buffer
- `len` — Data length

Returns the CRC-16 checksum.

## Example

### Single Checksum

```c
uint8_t msg[] = { 'x', 'b', 'o', 'o', 't' };
uint16_t checksum = crc16_sum(0, msg, sizeof(msg));
/* checksum = 0x394c */
```

### Chained Calls

```c
uint8_t part1[] = { 0x01, 0x02 };
uint8_t part2[] = { 0x03, 0x04 };
uint16_t crc = crc16_sum(0, part1, sizeof(part1));
crc = crc16_sum(crc, part2, sizeof(part2));
```
