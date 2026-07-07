# Complex Number (complex)

Single-precision floating-point complex number arithmetic for DSP.

## Data Structure

```c
struct complex_t {
    float real;
    float imag;
};
```

## API

```c
void complex_add(struct complex_t * z, struct complex_t * a, struct complex_t * b);
void complex_sub(struct complex_t * z, struct complex_t * a, struct complex_t * b);
void complex_mul(struct complex_t * z, struct complex_t * a, struct complex_t * b);
void complex_div(struct complex_t * z, struct complex_t * a, struct complex_t * b);
void complex_neg(struct complex_t * z, struct complex_t * a);
void complex_conj(struct complex_t * z, struct complex_t * a);
void complex_expj(struct complex_t * z, float phi);
void complex_proj(struct complex_t * z, struct complex_t * a);
void complex_normalize(struct complex_t * z, struct complex_t * a);
float complex_abs(struct complex_t * z);
float complex_arg(struct complex_t * z);
```

- `complex_add/sub/mul/div` — Arithmetic operations, `z = a op b`
- `complex_neg` — Negation, `z = -a`
- `complex_conj` — Conjugate, `z = a*`
- `complex_expj` — Rotation factor, `z = exp(j * phi)`
- `complex_proj` — Projection onto the Riemann sphere
- `complex_normalize` — Normalize to unit complex number
- `complex_abs` — Magnitude, `|z|`
- `complex_arg` — Phase angle, `arg(z)` in radians

## Example

### Complex Multiplication

```c
struct complex_t a = { 1.0f, 2.0f };
struct complex_t b = { 3.0f, 4.0f };
struct complex_t z;

complex_mul(&z, &a, &b);
```

### Rotation Factor

```c
struct complex_t z;
complex_expj(&z, 3.14159265f / 4.0f);
```

### Magnitude and Phase

```c
struct complex_t a = { 3.0f, 4.0f };
float r = complex_abs(&a);
float theta = complex_arg(&a);
```
