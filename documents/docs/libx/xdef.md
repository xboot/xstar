# 通用宏定义 (xdef)

`libx/xdef.h` 提供了 XSTAR 全局通用的基础宏定义和工具函数，包括常用尺寸常量、布尔/空指针定义、容器与偏移计算、分支预测提示、静态断言、数学辅助宏，以及位扫描内联函数。该头文件不依赖具体平台，所有源码均可包含。

## 尺寸常量

以十六进制形式预定义的常用内存尺寸常量，便于书写缓冲区大小、页面对齐、堆/栈尺寸等。

| 宏 | 值 | 含义 |
| --- | --- | --- |
| `SZ_16` | `0x00000010` | 16 字节 |
| `SZ_256` | `0x00000100` | 256 字节 |
| `SZ_512` | `0x00000200` | 512 字节 |
| `SZ_1K` | `0x00000400` | 1 KiB |
| `SZ_4K` | `0x00001000` | 4 KiB |
| `SZ_8K` | `0x00002000` | 8 KiB |
| `SZ_16K` | `0x00004000` | 16 KiB |
| `SZ_32K` | `0x00008000` | 32 KiB |
| `SZ_64K` | `0x00010000` | 64 KiB |
| `SZ_128K` | `0x00020000` | 128 KiB |
| `SZ_256K` | `0x00040000` | 256 KiB |
| `SZ_512K` | `0x00080000` | 512 KiB |
| `SZ_1M` | `0x00100000` | 1 MiB |
| `SZ_2M` | `0x00200000` | 2 MiB |
| `SZ_4M` | `0x00400000` | 4 MiB |
| `SZ_8M` | `0x00800000` | 8 MiB |
| `SZ_16M` | `0x01000000` | 16 MiB |
| `SZ_32M` | `0x02000000` | 32 MiB |
| `SZ_64M` | `0x04000000` | 64 MiB |
| `SZ_128M` | `0x08000000` | 128 MiB |
| `SZ_256M` | `0x10000000` | 256 MiB |
| `SZ_512M` | `0x20000000` | 512 MiB |
| `SZ_1G` | `0x40000000` | 1 GiB |
| `SZ_2G` | `0x80000000` | 2 GiB |

示例：

```c
char buf[SZ_4K];
void * heap = xos_mem_malloc(SZ_1M);
```

## 基础常量

### `NULL`

空指针常量。C++ 环境下定义为 `0`，C 环境下定义为 `((void *)0)`。仅在编译器未预先提供时定义。

### `TRUE` / `FALSE`

布尔真假常量，分别为 `1` 和 `0`。XSTAR 内部统一使用这两个宏来表示布尔结果，避免引入 `<stdbool.h>` 依赖。

```c
if(register_driver(&drv) == FALSE)
    return FALSE;
return TRUE;
```

## 结构与指针操作

### `offsetof(type, member)`

计算结构体成员相对于结构体起始位置的字节偏移。GCC 4 及以上版本使用编译器内建 `__builtin_offsetof`，否则使用经典宏定义。

```c
struct foo_t {
    int    a;
    char   b;
    void * c;
};

size_t off = offsetof(struct foo_t, c);
```

### `container_of(ptr, type, member)`

由结构体成员的地址反推得到包含它的结构体的地址。Linux 内核风格的经典宏，是 XSTAR 中实现侵入式数据结构（如 `list_head_t`、`hlist_node_t`、`rb_node_t` 等）的基础。

```c
struct list_head_t * pos;
struct my_node_t * node = container_of(pos, struct my_node_t, list);
```

## 分支预测

### `likely(expr)` / `unlikely(expr)`

提示编译器条件表达式的预期结果，便于优化指令布局。GCC 3 及以上版本使用 `__builtin_expect`，其他编译器退化为普通的布尔表达式。

```c
if(unlikely(ptr == NULL))
    return FALSE;

if(likely(count > 0))
    do_work();
```

> 仅在性能敏感且分支倾向非常明显的热路径上使用，普通代码不需要。

## 静态断言

### `STATIC_ASSERT(cond)`

编译期断言，条件为真时不产生任何代码，条件为假时通过定义大小为 `-1` 的数组触发编译错误。常用于约束结构体大小、枚举范围等。

```c
STATIC_ASSERT(sizeof(uint32_t) == 4);
STATIC_ASSERT(DEVICE_TYPE_MAX_COUNT < 256);
```

## 数学与数值工具

### `XMAP(x, ia, ib, oa, ob)`

将 `x` 从输入区间 `[ia, ib]` 线性映射到输出区间 `[oa, ob]`，常用于 ADC 采样、亮度/音量归一化等。

```c
int duty = XMAP(adc_value, 0, 4095, 0, 100);
```

### `XMIN(a, b)` / `XMAX(a, b)`

类型安全的最小值/最大值宏。使用 GCC 语句表达式与 `typeof`，避免参数副作用，并在类型不一致时给出编译告警。

```c
int m = XMIN(a, b);
int n = XMAX(x + 1, y - 1);
```

### `XCLAMP(v, a, b)`

将 `v` 限制到闭区间 `[a, b]`，等价于 `XMIN(XMAX(a, v), b)`。

```c
int volume = XCLAMP(user_input, 0, 100);
```

### `XFLOOR(x)` / `XROUND(x)` / `XCEIL(x)`

浮点向 `int` 取整宏，分别实现向下取整、四舍五入、向上取整。无需依赖 `<math.h>`，正负数处理对称。

```c
int a = XFLOOR(1.7);   /* 1 */
int b = XROUND(1.5);   /* 2 */
int c = XCEIL(-1.2);   /* -1 */
```

### `XDIV255(x)`

快速将整数除以 255，等价于 `(x + 1) * 257 >> 16`。常用于 8-bit 颜色通道的 alpha 混合计算，比常规除法更快。

```c
uint8_t blended = XDIV255((uint32_t)src * alpha);
```

### `XBYTEMUL(x, a)`

将 32 位整数中两组交错的 8-bit 字节通道（如 RGBA 的 R/B 与 G/A）同时乘以 8-bit 常量 `a` 并归一化到 8-bit，是图形子系统中颜色乘法的核心优化宏。

## 数组与字符串

### `ARRAY_SIZE(array)`

获取静态数组的元素个数。仅适用于真正的数组对象（非指针）。

```c
static const char * names[] = { "a", "b", "c" };
for(int i = 0; i < ARRAY_SIZE(names); i++)
    use(names[i]);
```

### `X(...)`

将可变参数原样转换为字符串字面量，常用于在不引入额外引号的情况下嵌入 JSON、设备树片段等多行文本。

```c
const char * json = X({
    "name": "demo",
    "value": 42
});
```

## 位扫描内联函数

下列内联函数均使用 GCC 内建指令实现，零开销。

### `int xffs(int x)`

返回 `x` 中最低位 `1` 的位置（从 1 开始计数），`x` 为 0 时返回 0。

### `int xfls(int x)`

返回 `x` 中最高位 `1` 的位置（从 1 开始计数），`x` 为 0 时返回 0。常用于计算以 2 为底的对数、向上取 2 的整数次幂。

```c
int order = xfls(size - 1);   /* 求最小 2^order >= size */
```

### `unsigned long __xffs(unsigned long x)`

返回 `x` 中最低位 `1` 的位下标（从 0 开始计数），等价于 `__builtin_ctzl`。`x` 不应为 0。

### `unsigned long __xfls(unsigned long x)`

返回 `x` 中最高位 `1` 的位下标（从 0 开始计数），等价于 `(sizeof(long) * 8) - 1 - __builtin_clzl(x)`。`x` 不应为 0。

```c
unsigned long bit = __xffs(mask);   /* 取出最低有效位下标 */
mask &= ~(1UL << bit);              /* 清除该位 */
```

## 使用建议

- 所有源文件应通过 `#include <xstar.h>` 或 `#include <libx/xdef.h>` 间接/直接引入这些定义。
- 优先使用 `XMIN`/`XMAX`/`XCLAMP` 而不是手写三元表达式，以避免参数副作用导致的难查 Bug。
- `likely`/`unlikely` 不要滥用；只有在确实需要优化指令布局的热点路径上才使用。
- `STATIC_ASSERT` 在头文件中可放置在文件作用域；在函数内部使用时同样生效。
- 与 XOS 抽象层、`libx/xtypes.h` 共同构成 XSTAR 源码所依赖的最小基础设施。
