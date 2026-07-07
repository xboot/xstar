# Clock Source (clocksource)

Clock source.

## Device Type

`DEVICE_TYPE_CLOCKSOURCE`

## Structure

```c
struct clocksource_t {
    struct clocksource_keeper_t keeper;
    char * name;
    uint64_t mask;
    uint32_t mult, shift;
    uint64_t (*read)(struct clocksource_t * cs);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_clocksource(name)` | Find a clock source by name |
| `register_clocksource(cs, drv)` | Register a clock source |
| `unregister_clocksource(cs)` | Unregister a clock source |
| `ktime_get()` | Get system kernel time |

## Description

Free-running monotonic counter, provides a continuous time base using a high-frequency hardware counter and scaling math.
