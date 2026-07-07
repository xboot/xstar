# libm (baremetal)

Baremetal 数学库实现，提供 C99 标准数学函数。适用于无操作系统的裸机环境，替代宿主系统的 libm。每个函数均提供 `double` 和 `float`（`f` 后缀）两个版本。

## 数学常量

| 宏 | 值 | 含义 |
|----|----|------|
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

## 特殊值

| 宏 | 含义 |
|----|------|
| `NAN` | 非数值 |
| `INFINITY` | 正无穷 |
| `HUGE_VALF` | float 正无穷 |
| `HUGE_VAL` | double 正无穷 |
| `HUGE_VALL` | long double 正无穷 |

## 浮点分类

| 宏 | 含义 |
|----|------|
| `fpclassify(x)` | 返回 `FP_NAN`/`FP_INFINITE`/`FP_ZERO`/`FP_SUBNORMAL`/`FP_NORMAL` |
| `isinf(x)` | 是否为无穷 |
| `isnan(x)` | 是否为 NaN |
| `isnormal(x)` | 是否为正常值 |
| `isfinite(x)` | 是否为有限值 |
| `signbit(x)` | 是否为负数 |

## 比较宏

| 宏 | 含义 |
|----|------|
| `isgreater(x, y)` | `x > y` |
| `isgreaterequal(x, y)` | `x >= y` |
| `isless(x, y)` | `x < y` |
| `islessequal(x, y)` | `x <= y` |
| `islessgreater(x, y)` | `x < y` 或 `x > y` |
| `isunordered(x, y)` | 任一为 NaN |

## 三角函数

```c
double acos(double);    float acosf(float);
double asin(double);    float asinf(float);
double atan(double);    float atanf(float);
double atan2(double, double);  float atan2f(float, float);
double cos(double);     float cosf(float);
double sin(double);     float sinf(float);
double tan(double);     float tanf(float);
```

## 双曲函数

```c
double acosh(double);   float acoshf(float);
double asinh(double);   float asinhf(float);
double atanh(double);   float atanhf(float);
double cosh(double);    float coshf(float);
double sinh(double);    float sinhf(float);
double tanh(double);    float tanhf(float);
```

## 指数与对数

```c
double exp(double);     float expf(float);
double exp2(double);    float exp2f(float);
double expm1(double);   float expm1f(float);
double log(double);     float logf(float);
double log10(double);   float log10f(float);
double log1p(double);   float log1pf(float);
double log2(double);    float log2f(float);
```

## 幂与根

```c
double pow(double, double);  float powf(float, float);
double sqrt(double);    float sqrtf(float);
double cbrt(double);    float cbrtf(float);
double hypot(double, double);  float hypotf(float, float);
```

## 误差函数

```c
double erf(double);     float erff(float);
double erfc(double);    float erfcf(float);
```

## 取整与余数

```c
double ceil(double);    float ceilf(float);
double floor(double);   float floorf(float);
double rint(double);    float rintf(float);
double round(double);   float roundf(float);
double trunc(double);   float truncf(float);
double fmod(double, double);  float fmodf(float, float);
double modf(double, double *);  float modff(float, float *);
```

## 绝对值、最大值、最小值

```c
double fabs(double);    float fabsf(float);
double fmax(double, double);  float fmaxf(float, float);
double fmin(double, double);  float fminf(float, float);
double fdim(double, double);  float fdimf(float, float);
```

## 浮点操作

```c
double frexp(double, int *);  float frexpf(float, int *);
double ldexp(double, int);    float ldexpf(float, int);
double scalbn(double, int);   float scalbnf(float, int);
double scalbln(double, long); float scalblnf(float, long);
```
