# 整数除法 (div)

软件整数除法，用于缺少硬件除法指令的平台（如部分 ARM 内核）。

## API

```c
uint32_t udiv32(uint32_t value, uint32_t divisor);
uint32_t umod32(uint32_t value, uint32_t divisor);
int32_t sdiv32(int32_t value, int32_t divisor);
int32_t smod32(int32_t value, int32_t divisor);
uint64_t udiv64(uint64_t value, uint64_t divisor);
uint64_t umod64(uint64_t value, uint64_t divisor);
int64_t sdiv64(int64_t value, int64_t divisor);
int64_t smod64(int64_t value, int64_t divisor);
```

- `udiv32/64` — 无符号除法
- `umod32/64` — 无符号取模
- `sdiv32/64` — 有符号除法
- `smod32/64` — 有符号取模

## 使用示例

### 32 位无符号除法

```c
uint32_t q = udiv32(100, 7);
uint32_t r = umod32(100, 7);
```

### 64 位有符号除法

```c
int64_t q = sdiv64(-1000000LL, 37LL);
int64_t r = smod64(-1000000LL, 37LL);
```
