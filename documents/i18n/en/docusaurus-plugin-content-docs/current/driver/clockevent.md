# Clock Event Timer (clockevent)

Clock event timer.

## Device Type

`DEVICE_TYPE_CLOCKEVENT`

## Structure

```c
struct clockevent_t {
    char * name;
    uint32_t mult, shift;
    uint64_t min_delta_ns, max_delta_ns;
    void * data;
    void (*handler)(struct clockevent_t * ce, void * data);
    int (*next)(struct clockevent_t * ce, uint64_t cycles);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_clockevent(name)` | Find a clock event by name |
| `register_clockevent(ce, drv)` | Register a clock event |
| `unregister_clockevent(ce)` | Unregister a clock event |
| `clockevent_set_event_handler(ce, handler, data)` | Set handler function |
| `clockevent_set_event_next(ce, cycles)` | Set next trigger |

## Description

Programmable timer event device. Uses scaling math to convert between counter ticks and nanoseconds.
