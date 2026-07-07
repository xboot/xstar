# Arctangent Lookup Table (atantab)

Inverse trigonometric function module based on a precomputed lookup table, providing both integer and floating-point precision. It is the inverse counterpart of the forward [Cosine Lookup Table (costab)](cosine-lookup-table.md), sharing the same 16-bit fixed-point angle convention, so results can be fed straight back into `isin()`/`icos()`.

## Principle

The forward table (costab) is indexed by a uniformly sampled angle and is naturally well-conditioned. Inverse trigonometric functions instead take a ratio as input, where `asin`/`acos` have an unbounded derivative as `|x| -> 1`; uniformly sampling the input value would incur huge errors near the endpoints.

To stay well-conditioned everywhere, the module only tabulates `atan(t)` (`t in [0, 1]`, whose derivative `1/(1+t^2)` is bounded in [0.5, 1]) and derives the rest via identities:

- `asin(x) = atan2(x, sqrt(1 - x^2))`
- `acos(x) = atan2(sqrt(1 - x^2), x)`

This pushes the singularity entirely onto the square root, keeping the table-lookup part benign everywhere.

`atantab[i] = atan(i / 1024)`, with 1025 entries (the last one being the `atan(1) = pi/4` endpoint, so linear interpolation at the top needs no clamping). Linear interpolation between adjacent samples yields accuracy far finer than the sample spacing.

`atan2` is implemented via first-quadrant reduction plus quadrant restoration: `|y|` and `|x|` are compared to squeeze the ratio into `[0, 1]`, the table yields an angle in `[0, pi/2]`, and the signs of `x` and `y` then map it into the correct quadrant.

## Lookup Tables

```c
extern const int16_t iatantab[1025];   /* Integer atan table, angle units [0, 8192] = [0, pi/4] */
extern const float    fatantab[1025];  /* Floating-point atan table, radians [0, pi/4] */
```

## API

### Integer Inverse Trigonometric Functions

```c
int iatan2(int y, int x);   /* Returns a 16-bit fixed-point angle, [-32768, 32767] = [-pi, pi) */
int iatan(int t);           /* t is a 1.10 fixed-point ratio (1024 == 1.0), returns [-8192, 8192] = [-pi/4, pi/4] */
int iasin(int v);           /* v is a unit-amplitude value (32767 == 1.0), returns a 16-bit fixed-point angle */
int iacos(int v);           /* v is a unit-amplitude value (32767 == 1.0), returns a 16-bit fixed-point angle */
```

Angles use 16-bit fixed-point units (2*pi == 65536), identical to `costab`, so the result of `iatan2()` can be passed directly to `isin()`/`icos()`. `iasin(v)`/`iacos(v)` take a unit-amplitude value in `[-32767, 32767]` (32767 corresponds to 1.0, matching `icostab`); the integer square root reuses `sqrti()`. `iatan2(0, 0)` is defined to return 0.

`iatan2()` accepts arbitrary integer inputs; magnitudes up to roughly 2*10^6 are safe with 32-bit intermediate arithmetic.

### Floating-Point Inverse Trigonometric Functions

```c
float fatan2(float y, float x);   /* Returns radians, [-pi, pi] */
float fatan(float t);             /* Clamped to |t| <= 1, returns [-pi/4, pi/4] */
float fasin(float x);             /* Clamped to |x| <= 1, returns [-pi/2, pi/2] */
float facos(float x);             /* Clamped to |x| <= 1, returns [0, pi] */
```

Inputs are standard floating-point values, angles are returned in radians. `fasin()`/`facos()` use `sqrtf()` internally.

### Angle Reference (same as costab)

| Angle value | Radians | Degrees |
|--------|------|------|
| 0x0000 | 0 | 0° |
| 0x4000 | pi/2 | 90° |
| 0x8000 | pi | 180° |
| 0xC000 | 3pi/2 | 270° |
| 0xFFFF | ~2pi | ~360° |

Radians to 16-bit fixed-point angle: `x = (int)(radians / (2 * PI) * 65536)`

## Accuracy

With 1024 samples plus linear interpolation, the measured maximum error (vs. the standard library) is:

| Function | Max error |
|----------|-----------|
| `fatan2` / `fatan` / `fasin` / `facos` | ~ 5x10^-7 rad (~ 0.00003°) |
| `iatan2` / `iasin` / `iacos` | ~ 1.6x10^-4 rad (~ 0.009°) |

The floating-point version reaches the float precision limit; the integer version is bottlenecked by the 16-bit angle output quantization (1~2 LSB), the table itself no longer being the limiting factor.

## Usage Examples

### Direction angle from coordinates (integer)

```c
int dx = target_x - origin_x;
int dy = target_y - origin_y;
int angle = iatan2(dy, dx);   /* 16-bit fixed-point angle, passable directly to isin()/icos() */
```

### Inverse-lookup of sine/cosine for an angle (closed loop with costab)

```c
int a = iatan2(1, 1);        /* 45° -> 0x2000 (8192) */
int s = isin(a);             /* ~ sin(45°) */
int c = icos(a);             /* ~ cos(45°) */
```

### Floating-point arcsine

```c
float a = fasin(0.5f);       /* ~ 0.5236 (30°) */
float b = facos(-1.0f);      /* ~ 3.1416 (180°) */
```

### Unit-amplitude integer arcsine

```c
int a = iasin(16384);        /* 16384/32767 ~ 0.5 -> about 0x1555 (30°) */
```
