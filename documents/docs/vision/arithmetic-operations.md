# 算术运算 (arithmetic-operations)

逐像素的按位逻辑运算。

## API

| 函数 | 说明 |
|------|------|
| `vision_bitwise_and(v, o)` | 与另一个图像按位与 |
| `vision_bitwise_or(v, o)` | 按位或 |
| `vision_bitwise_xor(v, o)` | 按位异或 |
| `vision_bitwise_not(v)` | 按位非 |

## 说明

所有操作直接修改 Vision 图像的像素数据。两图像尺寸必须匹配。
