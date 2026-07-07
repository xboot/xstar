# Clock (clk)

Clock.

## Device Type

`DEVICE_TYPE_CLK`

## Structure

```c
struct clk_t {
    char * name;
    int count;
    struct clk_t * (*set_parent)(struct clk_t * clk, struct clk_t * parent);
    struct clk_t * (*get_parent)(struct clk_t * clk);
    int (*set_enable)(struct clk_t * clk, int enable);
    int (*get_enable)(struct clk_t * clk);
    void (*set_rate)(struct clk_t * clk, uint64_t prate, uint64_t rate);
    uint64_t (*get_rate)(struct clk_t * clk, uint64_t prate);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_clk(name)` | Find a clock by name |
| `register_clk(clk, drv)` | Register a clock |
| `unregister_clk(clk)` | Unregister a clock |
| `clk_enable/disable(name)` | Enable/disable a clock |
| `clk_set_rate(name, rate)` | Set clock frequency |
| `clk_get_rate(name)` | Get clock frequency |

## Description

Clock tree management interface. Provides frequency configuration, parent source selection, and enable/disable gating.
