# 十六进制转储 (hexdump)

将内存数据以经典的 hex dump 格式输出，每行 16 字节，包含地址、十六进制值和 ASCII 字符。

## 输出格式

```
00000000: 48 65 6c 6c 6f 20 57 6f 72 6c 64 21 00 00 00 00 |Hello World!....|
00000010: 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 |................|
```

每行由三部分组成：
- **地址** — 8 位十六进制地址，基于 `base` 参数递增
- **十六进制** — 每字节 2 位十六进制 + 空格，不足 16 字节时对齐填充
- **ASCII** — 可打印字符（0x20~0x7e）原样显示，其余显示为 `.`

## API

```c
void hexdump(void (*output)(char), unsigned long base, void * buf, int len);
```

- `output` — 字符输出回调函数，每个字符调用一次，不可为 NULL
- `base` — 起始地址值（仅用于显示，不影响数据读取）
- `buf` — 待转储的数据缓冲区
- `len` — 数据长度（字节）

## 使用示例

### 输出到 shell

```c
static void shell_output(char c)
{
    shell_printf("%c", c);
}

hexdump(shell_output, 0x80000000, data, sizeof(data));
```

### 输出到动态字符串

```c
static struct ds_t * g_ds;

static void ds_output(char c)
{
    ds_append_char(g_ds, c);
}

g_ds = ds_alloc();
hexdump(ds_output, 0, buf, len);
/* g_ds 中包含完整的 hex dump 文本 */
ds_free(g_ds);
```

### 调试寄存器转储

```c
hexdump(shell_output, 0x40000000, mmio_base, 64);
/* 显示从 0x40000000 开始的 64 字节寄存器内容 */
```
