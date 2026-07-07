# Unaligned Access (unaligned)

Safely read or write multi-byte values from unaligned memory addresses, avoiding alignment exceptions on certain architectures.

## Background

Many RISC architectures (e.g., ARM, MIPS) require multi-byte accesses to be aligned (e.g., a 4-byte int must be 4-byte aligned), otherwise an exception is triggered. However, fields in protocol data and file formats are often unaligned. This module handles unaligned access safely by reading and writing byte by byte.

## Read Interface

```c
uint16_t get_unaligned_le16(const void * p);
uint16_t get_unaligned_be16(const void * p);
uint32_t get_unaligned_le32(const void * p);
uint32_t get_unaligned_be32(const void * p);
uint64_t get_unaligned_le64(const void * p);
uint64_t get_unaligned_be64(const void * p);
```

Read a value of the specified width from arbitrary address `p`, interpreting byte order as little-endian (le) or big-endian (be). `p` does not need to be aligned.

## Write Interface

```c
void put_unaligned_le16(void * p, uint16_t val);
void put_unaligned_be16(void * p, uint16_t val);
void put_unaligned_le32(void * p, uint32_t val);
void put_unaligned_be32(void * p, uint32_t val);
void put_unaligned_le64(void * p, uint64_t val);
void put_unaligned_be64(void * p, uint64_t val);
```

Write a value with the specified byte order to arbitrary address `p`. `p` does not need to be aligned.

## Differences from byteorder

| Module | Purpose | Alignment Requirement |
|--------|---------|----------------------|
| byteorder | CPU byte order conversion | Address must be aligned |
| unaligned | Unaligned address read/write + byte order conversion | Address does not need alignment |

`unaligned` handles byte order internally, so there is no need to call `byteorder` for conversion. It is suitable for directly extracting fields from a byte stream.

## Usage Examples

### Parsing a Protocol Header

```c
struct __attribute__((packed)) header {
    uint8_t cmd;
    uint8_t data[3];
};

void parse(const uint8_t * buf)
{
    uint8_t cmd = buf[0];
    uint32_t value = get_unaligned_le32(buf + 1);
    /* value is read starting from buf+1, which is not 4-byte aligned */
}
```

### Constructing a Data Packet

```c
uint8_t packet[10];
packet[0] = 0xAB;
put_unaligned_be16(packet + 1, 0x1234);
put_unaligned_be32(packet + 3, 0x56789ABC);
/* packet: AB 12 34 56 78 9A BC ... */
```
