# libm (baremetal)

Baremetal math library implementation, providing C99 standard math functions for use without an operating system. Each function has both `double` and `float` (`f` suffix) variants.

## Mathematical Constants

| Macro | Value | Meaning |
|-------|-------|---------|
| `M_E` | 2.71828... | e |
| `M_LOG2E` | 1.44269... | log2(e) |
| `M_LOG10E` | 0.43429... | log10(e) |
| `M_LN2` | 0.69314... | ln(2) |
| `M_LN10` | 2.30258... | ln(10) |
| `M_PI` | 3.14159... | π |
| `M_PI_2` | 1.57079... | π/2 |
| `M_PI_4` | 0.78539... | π/4 |
| `M_1_PI` | 0.31830... | 1/π |
| `M_2_PI` | 0.63661... | 2/π |
| `M_2_SQRTPI` | 1.12837... | 2/√π |
| `M_SQRT2` | 1.41421... | √2 |
| `M_SQRT1_2` | 0.70710... | 1/√2 |

## Special Values

| Macro | Meaning |
|-------|---------|
| `NAN` | Not a Number |
| `INFINITY` | Positive infinity |
| `HUGE_VALF` | float positive infinity |
| `HUGE_VAL` | double positive infinity |
| `HUGE_VALL` | long double positive infinity |

## FP Classification

| Macro | Meaning |
|-------|---------|
| `fpclassify(x)` | Returns `FP_NAN`/`FP_INFINITE`/`FP_ZERO`/`FP_SUBNORMAL`/`FP_NORMAL` |
| `isinf(x)` | Is infinite |
| `isnan(x)` | Is NaN |
| `isnormal(x)` | Is normal value |
| `isfinite(x)` | Is finite |
| `signbit(x)` | Is negative |

## Comparison Macros

| Macro | Meaning |
|-------|---------|
| `isgreater(x, y)` | `x > y` |
| `isgreaterequal(x, y)` | `x >= y` |
| `isless(x, y)` | `x < y` |
| `islessequal(x, y)` | `x <= y` |
| `islessgreater(x, y)` | `x < y` or `x > y` |
| `isunordered(x, y)` | Either is NaN |

## Trigonometric Functions

```c
double acos(double);    float acosf(float);
double asin(double);    float asinf(float);
double atan(double);    float atanf(float);
double atan2(double, double);  float atan2f(float, float);
double cos(double);     float cosf(float);
double sin(double);     float sinf(float);
double tan(double);     float tanf(float);
```

## Hyperbolic Functions

```c
double acosh(double);   float acoshf(float);
double asinh(double);   float asinhf(float);
double atanh(double);   float atanhf(float);
double cosh(double);    float coshf(float);
double sinh(double);    float sinhf(float);
double tanh(double);    float tanhf(float);
```

## Exponential and Logarithmic Functions

```c
double exp(double);     float expf(float);
double exp2(double);    float exp2f(float);
double expm1(double);   float expm1f(float);
double log(double);     float logf(float);
double log10(double);   float log10f(float);
double log1p(double);   float log1pf(float);
double log2(double);    float log2f(float);
```

## Power and Root Functions

```c
double pow(double, double);  float powf(float, float);
double sqrt(double);    float sqrtf(float);
double cbrt(double);    float cbrtf(float);
double hypot(double, double);  float hypotf(float, float);
```

## Error Functions

```c
double erf(double);     float erff(float);
double erfc(double);    float erfcf(float);
```

## Rounding and Remainder

```c
double ceil(double);    float ceilf(float);
double floor(double);   float floorf(float);
double rint(double);    float rintf(float);
double round(double);   float roundf(float);
double trunc(double);   float truncf(float);
double fmod(double, double);  float fmodf(float, float);
double modf(double, double *);  float modff(float, float *);
```

## Absolute Value, Min, Max

```c
double fabs(double);    float fabsf(float);
double fmax(double, double);  float fmaxf(float, float);
double fmin(double, double);  float fminf(float, float);
double fdim(double, double);  float fdimf(float, float);
```

## Floating-Point Manipulation

```c
double frexp(double, int *);  float frexpf(float, int *);
double ldexp(double, int);    float ldexpf(float, int);
double scalbn(double, int);   float scalbnf(float, int);
double scalbln(double, long); float scalblnf(float, long);
```
