# md

Dump memory region contents.

## Usage

```
md [-b|-w|-l|-q] address [-c=count]
```

## Options

| Option | Description |
|--------|-------------|
| `-b` | Byte (8-bit, default) |
| `-w` | Word (16-bit) |
| `-l` | Long (32-bit) |
| `-q` | Quad (64-bit) |
| `-c=count` | Number of elements to display (default 64) |

## Notes

- Displays hexadecimal values and ASCII representation
- Respects alignment constraints of the access size
- Supports Ctrl-C to abort

## Examples

```bash
# Display 64 bytes as bytes
md -b 0x80000000

# Display 16 values as longs
md -l 0x80000000 -c=16
```
