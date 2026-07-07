# mw

Write values to a memory region.

## Usage

```
mw [-b|-w|-l|-q] address value [-c=count]
```

## Options

| Option | Description |
|--------|-------------|
| `-b` | Byte (8-bit, default) |
| `-w` | Word (16-bit) |
| `-l` | Long (32-bit) |
| `-q` | Quad (64-bit) |
| `-c=count` | Number of writes (default 1) |

## Notes

- Writes the same value repeatedly to contiguous physical memory
- Respects alignment constraints for the access size
- Supports Ctrl-C to abort

## Examples

```bash
# Write a single byte
mw -b 0x80000000 0xff

# Write 16 long words
mw -l 0x80000000 0xdeadbeef -c=16
```
