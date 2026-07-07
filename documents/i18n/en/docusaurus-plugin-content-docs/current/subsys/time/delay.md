# Delay (delay)

Busy-wait delay functions based on the clock source (clocksource), providing nanosecond, microsecond, and millisecond precision delays, suitable for short and precise wait scenarios.

## How It Works

The delay functions obtain the current time via `ktime_get()`, compute the target timeout, then busy-wait poll until the timeout:

```c
void udelay(uint32_t us)
{
    ktime_t timeout = ktime_add_us(ktime_get(), us);
    while(ktime_before(ktime_get(), timeout));
}
```

All three delay functions share the same implementation, differing only in time unit:

- `ndelay()` uses `ktime_add_ns()` to add nanoseconds
- `udelay()` uses `ktime_add_us()` to add microseconds
- `mdelay()` uses `ktime_add_ms()` to add milliseconds

## API

| Function | Description |
|----------|-------------|
| `ndelay(ns)` | Nanosecond busy-wait delay |
| `udelay(us)` | Microsecond busy-wait delay |
| `mdelay(ms)` | Millisecond busy-wait delay |

## Usage Examples

```c
#include <kernel/time/delay.h>

/* Wait 10 microseconds after hardware reset */
udelay(10);

/* I2C timing delay */
ndelay(500);

/* Short wait of 5 milliseconds */
mdelay(5);
```

## Notes

- Delays are busy-waits; the CPU is not released during the wait, making them unsuitable for long waits
- Delay precision depends on the clock source device resolution; nanosecond-level timestamps are obtained via `ktime_get()`
- For long waits, use the timer system (`timer`) or thread sleep (`xos_thread_msleep`)
- Safe to use in interrupt context since no scheduling is involved
