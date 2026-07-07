# 设备树 (dtree)

设备树节点抽象层，从 JSON 格式的设备树数据中读取配置。驱动程序使用此接口从 boot.json 读取配置。

## 数据结构

```c
struct dtnode_t {
    const char * name;
    int id;
    uint64_t addr;
    struct json_value_t * value;
};
```

## API

### 节点信息

```c
const char * dt_read_name(struct dtnode_t * n);
int dt_read_id(struct dtnode_t * n);
uint64_t dt_read_address(struct dtnode_t * n);
```

### 节点命名规则

设备树 JSON 的每个 key 使用 `name[:id][@address]` 格式编码节点信息，其中 `:` 和 `@` 均为可选分隔符：

| 分隔符 | 字段 | 说明 |
| --- | --- | --- |
| `:` | `id` | 驱动实例编号，通过 `dt_read_id()` 读取（十进制/十六进制）；同一种驱动有多个实例时用于区分 |
| `@` | `addr` | 硬件基址，通过 `dt_read_address()` 读取（支持 `0x` 前缀）；用于 MMIO 外设的地址映射 |

`:` 与 `@` 出现的顺序不限，可以同时使用。解析时以第一个遇到的 `:` 或 `@` 为界划分。

```json
{
    "clk-fixed":                    { ... },   /* name="clk-fixed",         id=0, addr=0          */
    "clk-gate@0x07090000":          { ... },   /* name="clk-gate",          id=0, addr=0x07090000 */
    "clk-f133-pll:0":               { ... },   /* name="clk-f133-pll",      id=0, addr=0          */
    "irq-rk3506-gpio:0@0xff940000": { ... },   /* name="irq-rk3506-gpio",   id=0, addr=0xff940000 */
}
```

驱动 probe 内部通常会调用 `alloc_device_name(dt_read_name(n), dt_read_id(n))` 为设备分配全局唯一名称（如 `clk-f133-pll.0`、`clk-f133-pll.1`），确保同一驱动多个实例的独立标识。

### 标量读取

```c
int dt_read_bool(struct dtnode_t * n, const char * name, int def);
int dt_read_int(struct dtnode_t * n, const char * name, int def);
long long dt_read_long(struct dtnode_t * n, const char * name, long long def);
double dt_read_double(struct dtnode_t * n, const char * name, double def);
char * dt_read_string(struct dtnode_t * n, const char * name, char * def);
uint8_t dt_read_u8(struct dtnode_t * n, const char * name, uint8_t def);
uint16_t dt_read_u16(struct dtnode_t * n, const char * name, uint16_t def);
uint32_t dt_read_u32(struct dtnode_t * n, const char * name, uint32_t def);
uint64_t dt_read_u64(struct dtnode_t * n, const char * name, uint64_t def);
struct dtnode_t * dt_read_object(struct dtnode_t * n, const char * name, struct dtnode_t * o);
```

### 数组读取

```c
int dt_read_array_length(struct dtnode_t * n, const char * name);
int dt_read_array_bool(struct dtnode_t * n, const char * name, int idx, int def);
int dt_read_array_int(struct dtnode_t * n, const char * name, int idx, int def);
long long dt_read_array_long(struct dtnode_t * n, const char * name, int idx, long long def);
double dt_read_array_double(struct dtnode_t * n, const char * name, int idx, double def);
char * dt_read_array_string(struct dtnode_t * n, const char * name, int idx, char * def);
uint8_t dt_read_array_u8(struct dtnode_t * n, const char * name, int idx, uint8_t def);
uint16_t dt_read_array_u16(struct dtnode_t * n, const char * name, int idx, uint16_t def);
uint32_t dt_read_array_u32(struct dtnode_t * n, const char * name, int idx, uint32_t def);
uint64_t dt_read_array_u64(struct dtnode_t * n, const char * name, int idx, uint64_t def);
struct dtnode_t * dt_read_array_object(struct dtnode_t * n, const char * name, int idx, struct dtnode_t * o);
```

所有读取函数均带 `def` 默认参数，属性不存在时安全返回默认值。

### 特殊属性

| 属性 | 说明 |
| --- | --- |
| `"status": "disabled"` | 节点被标记为禁用，加载时跳过 `probe`，等价于直接从设备树中删除该节点 |

## 使用示例

### 驱动 probe 中读取配置

```c
static struct device_t * clk_fixed_probe(struct driver_t * drv, struct dtnode_t * n)
{
    const char * name = dt_read_name(n);
    uint32_t rate = dt_read_u32(n, "clock-frequency", 0);
    int enable = dt_read_bool(n, "clock-enable", 0);
}
```

### 读取子对象

```c
struct dtnode_t child;
dt_read_object(n, "sub-device", &child);
uint32_t addr = dt_read_u32(&child, "reg", 0);
```

### 读取数组

```c
int len = dt_read_array_length(n, "pins");
for(int i = 0; i < len; i++)
{
    uint32_t pin = dt_read_array_u32(n, "pins", i, 0);
}
```
