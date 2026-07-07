# Byte Order Conversion (byteorder)

Conversion interface between CPU byte order and little-endian/big-endian formats, used for cross-platform scenarios such as protocol parsing and file format handling.

## Background

Different CPU architectures have different byte orders (x86 is little-endian, ARM is configurable), while network protocols use big-endian (network byte order), and many file formats also specify byte order. This module provides a unified conversion interface, eliminating platform differences.

## Byte Order Detection

```c
int cpu_is_big_endian(void);
```

Runtime detection of whether the current CPU is big-endian. Returns 1 for big-endian, 0 for little-endian.

## Conversion Interfaces

All interfaces are `static inline`, with zero additional call overhead.

### CPU ↔ Little-Endian

| Interface | Description |
|------|------|
| `cpu_to_le16(x)` | CPU byte order → 16-bit little-endian |
| `le16_to_cpu(x)` | 16-bit little-endian → CPU byte order |
| `cpu_to_le32(x)` | CPU byte order → 32-bit little-endian |
| `le32_to_cpu(x)` | 32-bit little-endian → CPU byte order |
| `cpu_to_le64(x)` | CPU byte order → 64-bit little-endian |
| `le64_to_cpu(x)` | 64-bit little-endian → CPU byte order |

On little-endian systems, `cpu_to_le` and `le_to_cpu` are identity operations; on big-endian systems, they perform byte swapping. Conversions in the same direction are inverse operations: `le32_to_cpu(cpu_to_le32(x)) == x`.

### CPU ↔ Big-Endian

| Interface | Description |
|------|------|
| `cpu_to_be16(x)` | CPU byte order → 16-bit big-endian |
| `be16_to_cpu(x)` | 16-bit big-endian → CPU byte order |
| `cpu_to_be32(x)` | CPU byte order → 32-bit big-endian |
| `be32_to_cpu(x)` | 32-bit big-endian → CPU byte order |
| `cpu_to_be64(x)` | CPU byte order → 64-bit big-endian |
| `be64_to_cpu(x)` | 64-bit big-endian → CPU byte order |

Identity operation on big-endian systems, byte swapping on little-endian systems.

### Generic Byte Swapping

| Interface | Description |
|------|------|
| `__swab16(x)` | 16-bit byte swap |
| `__swab32(x)` | 32-bit byte swap |
| `__swab64(x)` | 64-bit byte swap |
| `__swahw32(x)` | 32-bit half-word swap (swap high and low 16 bits) |
| `__swahb32(x)` | 32-bit half-word byte swap (swap two bytes within each 16 bits) |

## Usage Examples

### Parsing Network Packets

```c
uint16_t port = be16_to_cpu(*(uint16_t *)packet->port_field);
uint32_t addr = be32_to_cpu(*(uint32_t *)packet->addr_field);
```

### Writing Little-Endian File Format

```c
*(uint32_t *)buf = cpu_to_le32(value);
*(uint16_t *)(buf + 4) = cpu_to_le16(count);
```
