# BCD Encoding (bcd)

Conversion interface between BCD (Binary-Coded Decimal) and binary, used for interacting with hardware such as RTC chips that use BCD encoding.

## Principle

BCD encoding uses 4 bits to represent 1 decimal digit. The high 4 bits of a byte represent the tens digit, and the low 4 bits represent the units digit.

```
Decimal 25 → BCD 0x25 (0010 0101)
Decimal 59 → BCD 0x59 (0101 1001)
```

| Decimal | Binary | BCD |
|--------|--------|-----|
| 0 | 0x00 | 0x00 |
| 12 | 0x0C | 0x12 |
| 59 | 0x3B | 0x59 |
| 99 | 0x63 | 0x99 |

Note: BCD value `0x59` is not equal to decimal `89`, it represents decimal `59`.

## API

```c
unsigned char bcd2bin(unsigned char x);
```

BCD to binary. For example `0x59` → `59`.

```c
unsigned char bin2bcd(unsigned char x);
```

Binary to BCD. For example `59` → `0x59`.

Both functions are `static inline`, with zero additional call overhead. Input range 0~99.

## Usage Examples

### Reading RTC Time

```c
unsigned char bcd_sec = rtc_read_sec();
unsigned char sec = bcd2bin(bcd_sec);
```

### Writing RTC Time

```c
unsigned char sec = 30;
rtc_write_sec(bin2bcd(sec));
```
