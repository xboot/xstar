# CRC-8校验 (crc8)

CRC-8 循环冗余校验，采用 **CRC-8/SMBUS** 算法标准。可链式调用：将上一次的 CRC 输出作为下一次的初始值。

## 算法参数

| 参数 | 值 |
|------|------|
| 名称 | CRC-8/SMBUS |
| 多项式 | `0x07` (x⁸ + x² + x + 1) |
| 初始值 | `0x00` |
| 反转输入 | 否 |
| 反转输出 | 否 |
| 最终异或 | `0x00` |
| 校验值 | `"123456789"` → `0xF4` |

## API

```c
uint8_t crc8_sum(uint8_t crc, const uint8_t * buf, int len);
```

- `crc` — 初始值，首次调用通常传 `0`
- `buf` — 数据缓冲区
- `len` — 数据长度

返回 CRC-8 校验值。

## 使用示例

### 单次校验

```c
uint8_t msg[] = { 'x', 'b', 'o', 'o', 't' };
uint8_t checksum = crc8_sum(0, msg, sizeof(msg));
/* checksum = 0x01 */
```

### 链式调用

```c
uint8_t part1[] = { 0x01, 0x02 };
uint8_t part2[] = { 0x03, 0x04 };
uint8_t crc = crc8_sum(0, part1, sizeof(part1));
crc = crc8_sum(crc, part2, sizeof(part2));
```
