# 复数运算 (complex)

单精度浮点复数运算，用于数字信号处理。

## 数据结构

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

- `complex_add/sub/mul/div` — 四则运算，`z = a op b`
- `complex_neg` — 取负，`z = -a`
- `complex_conj` — 共轭，`z = a*`
- `complex_expj` — 旋转因子，`z = exp(j * phi)`
- `complex_proj` — 投影到黎曼球面
- `complex_normalize` — 归一化为单位复数
- `complex_abs` — 模，`|z|`
- `complex_arg` — 幅角，`arg(z)`（弧度）

## 使用示例

### 复数乘法

```c
struct complex_t a = { 1.0f, 2.0f };
struct complex_t b = { 3.0f, 4.0f };
struct complex_t z;

complex_mul(&z, &a, &b);
```

### 旋转因子

```c
struct complex_t z;
complex_expj(&z, 3.14159265f / 4.0f);
```

### 求模与幅角

```c
struct complex_t a = { 3.0f, 4.0f };
float r = complex_abs(&a);
float theta = complex_arg(&a);
```
