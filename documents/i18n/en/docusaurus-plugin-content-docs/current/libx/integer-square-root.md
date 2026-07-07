# Integer Square Root (sqrti)

Integer square root computation without floating-point operations, suitable for embedded platforms without an FPU.

## Algorithm

Based on the digit-by-digit method, determining each bit of the square root starting from the most significant bit. Similar to long division for square roots, all operations are integer addition, subtraction, and shifts, with no floating-point dependency.

## API

```c
unsigned long sqrti(unsigned long x);
```

Computes the integer square root of `x`, returning `⌊√x⌋`.

- `x = 0` returns 0
- `x = 1` returns 1
- `x = 4` returns 2
- `x = 5` returns 2 (rounded down)

## Usage Examples

### Calculate distance (Pythagorean theorem)

```c
unsigned long dx = x2 - x1;
unsigned long dy = y2 - y1;
unsigned long dist = sqrti(dx * dx + dy * dy);
```

### Graphics scaling

```c
unsigned long ratio = sqrti(src_size * src_size / dst_size);
```
