# dcp

Data copy between devices (file, block device, memory).

## Usage

```
dcp <input@offset:size> <output@offset:size>
```

## Description

- Supports arbitrary combinations of copying between block devices, XFS files, and raw memory
- Format: `<name>@<offset>:<size>`, omitting the name indicates raw memory access
- Offset and size support decimal and hexadecimal (`0x` prefix)
- Transfers using 64KB buffer
- Displays transfer speed and total amount

## Examples

```bash
# Copy from memory to file
dcp @0x80000000:0x1000 /tmp/dump.bin@0:0x1000

# Copy from block device to file
dcp blk.0@0:0x10000 /tmp/backup.bin@0:0x10000
```
