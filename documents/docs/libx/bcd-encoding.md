# BCD 编码 (bcd)

BCD（Binary-Coded Decimal）与二进制之间的转换接口，用于 RTC 芯片等使用 BCD 编码的硬件交互。

## 原理

BCD 编码用 4 位二进制表示 1 位十进制数。一个字节的高 4 位表示十位，低 4 位表示个位。

```
十进制 25 → BCD 0x25 (0010 0101)
十进制 59 → BCD 0x59 (0101 1001)
```

| 十进制 | 二进制 | BCD |
|--------|--------|-----|
| 0 | 0x00 | 0x00 |
| 12 | 0x0C | 0x12 |
| 59 | 0x3B | 0x59 |
| 99 | 0x63 | 0x99 |

注意：BCD 值 `0x59` 不等于十进制 `89`，它表示十进制 `59`。

## API

```c
unsigned char bcd2bin(unsigned char x);
```

BCD 转二进制。例如 `0x59` → `59`。

```c
unsigned char bin2bcd(unsigned char x);
```

二进制转 BCD。例如 `59` → `0x59`。

两个函数均为 `static inline`，零额外调用开销。输入范围 0~99。

## 使用示例

### 读取 RTC 时间

```c
unsigned char bcd_sec = rtc_read_sec();
unsigned char sec = bcd2bin(bcd_sec);
```

### 写入 RTC 时间

```c
unsigned char sec = 30;
rtc_write_sec(bin2bcd(sec));
```
