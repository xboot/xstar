# Arithmetic Operations (arithmetic-operations)

Per-pixel bitwise logic operations.

## API

| Function | Description |
|------|------|
| `vision_bitwise_and(v, o)` | Bitwise AND with another image |
| `vision_bitwise_or(v, o)` | Bitwise OR |
| `vision_bitwise_xor(v, o)` | Bitwise XOR |
| `vision_bitwise_not(v)` | Bitwise NOT |

## Notes

All operations directly modify the pixel data of the Vision image. Both images must have matching dimensions.

