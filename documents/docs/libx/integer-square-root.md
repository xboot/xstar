# 整数平方根 (sqrti)

无浮点运算的整数平方根计算，适用于无 FPU 的嵌入式平台。

## 算法

基于逐位逼近法（digit-by-digit），从最高有效位开始逐位确定平方根的每一位。类似长除法求平方根，所有运算为整数加减和移位，无浮点依赖。

## API

```c
unsigned long sqrti(unsigned long x);
```

计算 `x` 的整数平方根，返回 `⌊√x⌋`。

- `x = 0` 返回 0
- `x = 1` 返回 1
- `x = 4` 返回 2
- `x = 5` 返回 2（向下取整）

## 使用示例

### 计算距离（勾股定理）

```c
unsigned long dx = x2 - x1;
unsigned long dy = y2 - y1;
unsigned long dist = sqrti(dx * dx + dy * dy);
```

### 图形缩放

```c
unsigned long ratio = sqrti(src_size * src_size / dst_size);
```
