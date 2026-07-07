# Device Tree (dtree)

Device tree node abstraction layer for reading configuration from JSON-based device tree data. Primary interface for drivers to read configuration from boot.json.

## Data Structure

```c
struct dtnode_t {
    const char * name;
    int id;
    uint64_t addr;
    struct json_value_t * value;
};
```

## API

### Node Information

```c
const char * dt_read_name(struct dtnode_t * n);
int dt_read_id(struct dtnode_t * n);
uint64_t dt_read_address(struct dtnode_t * n);
```

### Node Naming Convention

Each JSON key in the device tree encodes node information using the format `name[:id][@address]`, where `:` and `@` are optional separators:

| Separator | Field | Description |
| --- | --- | --- |
| `:` | `id` | Driver instance number, returned by `dt_read_id()` (decimal or hexadecimal); used to distinguish multiple instances of the same driver |
| `@` | `addr` | Hardware base address, returned by `dt_read_address()` (accepts `0x` prefix); used for MMIO peripheral address mapping |

`:` and `@` can appear in any order and may be combined. The parser splits on the first occurrence of either character.

```json
{
    "clk-fixed":                    { ... },   /* name="clk-fixed",        id=0, addr=0          */
    "clk-gate@0x07090000":          { ... },   /* name="clk-gate",         id=0, addr=0x07090000 */
    "clk-f133-pll:0":               { ... },   /* name="clk-f133-pll",     id=0, addr=0          */
    "irq-rk3506-gpio:0@0xff940000": { ... },   /* name="irq-rk3506-gpio",  id=0, addr=0xff940000 */
}
```

Inside a driver probe, the conventional call `alloc_device_name(dt_read_name(n), dt_read_id(n))` is used to mint a globally unique device name (such as `clk-f133-pll.0`, `clk-f133-pll.1`), ensuring distinct identities for multiple instances of the same driver.

### Scalar Reads

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

### Array Reads

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

All read functions take a `def` default parameter and safely return the default value when the property does not exist.

### Special Properties

| Property | Description |
| --- | --- |
| `"status": "disabled"` | The node is marked as disabled; `probe` is skipped at load time, equivalent to removing the node from the device tree entirely |

## Example

### Reading Configuration in Driver Probe

```c
static struct device_t * clk_fixed_probe(struct driver_t * drv, struct dtnode_t * n)
{
    const char * name = dt_read_name(n);
    uint32_t rate = dt_read_u32(n, "clock-frequency", 0);
    int enable = dt_read_bool(n, "clock-enable", 0);
}
```

### Reading Sub-Object

```c
struct dtnode_t child;
dt_read_object(n, "sub-device", &child);
uint32_t addr = dt_read_u32(&child, "reg", 0);
```

### Reading Array

```c
int len = dt_read_array_length(n, "pins");
for(int i = 0; i < len; i++)
{
    uint32_t pin = dt_read_array_u32(n, "pins", i, 0);
}
```
