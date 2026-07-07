# CRC-32校验 (crc32)

CRC-32 循环冗余校验，采用 **CRC-32 (ISO 3309)** 算法标准。可链式调用：将上一次的 CRC 输出作为下一次的初始值。

## 算法参数

| 参数 | 值 |
|------|------|
| 名称 | CRC-32 (ISO 3309 / ITU-T V.42) |
| 多项式 | `0x04C11DB7` |
| 初始值 | `0xFFFFFFFF` |
| 反转输入 | 是 |
| 反转输出 | 是 |
| 最终异或 | `0xFFFFFFFF` |
| 校验值 | `"123456789"` → `0xCBF43926` |

## API

```c
uint32_t crc32_sum(uint32_t crc, const uint8_t * buf, int len);
```

- `crc` — 初始值，首次调用通常传 `0`
- `buf` — 数据缓冲区
- `len` — 数据长度

返回 CRC-32 校验值。

## 使用示例

### 单次校验

```c
uint8_t msg[] = { 'x', 'b', 'o', 'o', 't' };
uint32_t checksum = crc32_sum(0, msg, sizeof(msg));
/* checksum = 0x68292dcb */
```

### 链式调用

```c
uint8_t part1[] = { 0x01, 0x02 };
uint8_t part2[] = { 0x03, 0x04 };
uint32_t crc = crc32_sum(0, part1, sizeof(part1));
crc = crc32_sum(crc, part2, sizeof(part2));
```
