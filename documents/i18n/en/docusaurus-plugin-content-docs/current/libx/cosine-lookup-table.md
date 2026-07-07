# Cosine Lookup Table (costab)

Trigonometric function module based on a precomputed lookup table, providing both integer and floating-point precision, using 16-bit fixed-point angle representation with zero floating-point operation overhead.

## Principle

Uniformly sample 0 to 2π into 65536 angle units (0x0000 to 0xFFFF), precompute 1024 cosine values in the [0, π/2] interval as the lookup table. Other quadrants are mapped via symmetry:

- `cos(x)` — direct table lookup
- `sin(x)` — using `sin(x) = cos(x - π/2)`, add `0xC000` to the angle (i.e. 3π/2, equivalent to -π/2 mod 2π) before performing the table lookup

Angle values automatically wrap modulo 65536, no manual period handling required.

## Lookup Tables

```c
extern const int16_t icostab[1024];   /* Integer cosine table, range [-32767, 32767] */
extern const float fcostab[1024];      /* Floating-point cosine table, range [-1.0, 1.0] */
```

## API

### Integer Trigonometric Functions

```c
int isin(int x);   /* Returns [-32767, 32767] */
int icos(int x);   /* Returns [-32767, 32767] */
```

Input is a 16-bit fixed-point angle (0 to 65535 represents 0 to 2π). Output is a 15-bit signed fixed-point number, where 32767 corresponds to 1.0.

### Floating-Point Trigonometric Functions

```c
float fsin(int x);   /* Returns [-1.0, 1.0] */
float fcos(int x);   /* Returns [-1.0, 1.0] */
```

Input is the same 16-bit fixed-point angle, output is a standard floating-point value.

### Angle Reference

| Angle value | Radians | Degrees |
|--------|------|------|
| 0x0000 | 0 | 0° |
| 0x4000 | π/2 | 90° |
| 0x8000 | π | 180° |
| 0xC000 | 3π/2 | 270° |
| 0xFFFF | ≈2π | ≈360° |

Angle conversion formula: `x = (int)(radians / (2 * PI) * 65536)`

## Usage Examples

### Integer Precision Rotation Coordinates

```c
int angle = 0x2000;  /* 45 degrees */
int x = 100, y = 0;
int nx = (x * icos(angle) - y * isin(angle)) >> 15;
int ny = (x * isin(angle) + y * icos(angle)) >> 15;
```

### Floating-Point Precision Calculation

```c
float s = fsin(0x4000);  /* ≈ 1.0 (sin(90°)) */
float c = fcos(0x0000);  /* ≈ 1.0 (cos(0°)) */
```

### Incremental Angle Animation

```c
int angle = 0;
while(1)
{
    int dx = isin(angle);
    int dy = icos(angle);
    angle += 0x0100;  /* approximately 5.6 degrees per step */
    angle &= 0xFFFF;  /* automatic wrap-around */
}
```
