# dcp

Copy data between devices, supporting any combination of block devices, XFS files, and raw memory.

## Usage

```
dcp <input@offset:size> <output@offset:size>
```

## Argument Format

Both input and output use the `<name>@<offset>:<size>` format, and every field is optional:

| Field | Description | Default |
|-------|-------------|---------|
| `name` | Block device name or XFS file path; omit for raw memory | none (raw memory) |
| `offset` | Start offset, decimal or hexadecimal (`0x` prefix) | `0` |
| `size` | Number of bytes to copy, decimal or hexadecimal | to the end of the device |

## Device Type Resolution

The `name` field is resolved in the following order:

1. First matched as a **block device** name (e.g. `blk.0`, including partition devices such as `blk.0.xstar`)
2. If not found, resolved as an **XFS file** path (requires a mounted filesystem)
3. When `name` is omitted, the argument refers to **raw memory**, and `offset` is the memory address

## Notes

- The actual copied size is the smaller of the input available size and the output size
- The input block device is synced before copying; the output block device or file is automatically flushed afterwards
- When the output is a file, `offset` is ignored and writing always starts at the beginning of the file
- Transfers in 64KB chunks, reporting the total amount and speed when finished

## Caveats

- Raw memory access performs no bounds checking; the caller must ensure the address and size are valid
- Writing to a block device is a raw write: NAND devices (e.g. spinand) perform read-modify-write per erase block, so do not overwrite system partitions by mistake

## Examples

```bash
# Dump memory to a file
dcp @0x80000000:0x1000 /tmp/dump.bin@0:0x1000

# Back up a block device partition to a file
dcp blk.0.xstar@0:0x10000 /tmp/backup.bin@0:0x10000

# Flash an image file to a block device
dcp /tmp/image.bin@0:0x100000 blk.0@0x10000:0x100000

# Read block device content back into memory, inspect with the md command
dcp blk.0@0x10000:0x100 @0x40300000:0x100

# Copy between partitions
dcp blk.0.private@0:0x10000 blk.1@0:0x10000

# Omit size to copy from an offset to the end of the device
dcp blk.0@0x1000 /tmp/full.bin
```
