# Limit Switch (limiter)

Limit switch / endstop subsystem. Returns a binary status indicating whether the mechanical limit has been reached. Commonly used in CNC, 3D printers, robotics, automatic doors, level sensing, etc.

## Driver Framework

### Device Type

`DEVICE_TYPE_LIMITER`

### Core Data Structure

```c
struct limiter_t {
    char * name;
    int (*get)(struct limiter_t * limiter);
    void * priv;
};
```

- `name`: Globally unique device name, lookable via `search_limiter()`.
- `get`: Status read callback implemented by the concrete driver. Returns `1` when the limit is triggered, `0` otherwise.
- `priv`: Private data pointer for the concrete driver.

### Core API

| Function | Description |
|------|------|
| `search_limiter(name)` | Find limiter by name |
| `register_limiter(limiter, drv)` | Register a limiter; the sysfs node is created automatically |
| `unregister_limiter(limiter)` | Unregister a limiter |
| `limiter_get_status(limiter)` | Get current limit status (active query) |

### sysfs Node

Upon registration the core layer automatically creates a read-only `status` node, which can be inspected in real time via the kobj virtual file system in the debug shell.

### Event Notification Mechanism

The limiter subsystem uses the **psub (publish/subscribe)** pattern to asynchronously notify upper-layer applications about state changes, eliminating the need for active polling from the application side.

#### Event Topics

| Topic | Triggered When | Data |
|-------|----------------|------|
| `limiter.triggered` | Limit is triggered (status changes from 0 to 1) | `struct limiter_t *` |
| `limiter.released`  | Limit is released  (status changes from 1 to 0) | `struct limiter_t *` |

The event payload is a `struct limiter_t *` pointer, so subscribers can distinguish between multiple limiters by `limiter->name`.

#### Subscriber Example (Application)

Upper-layer applications register callbacks via `psub_subscribe()` to receive events:

```c
#include <kernel/core/psub.h>
#include <driver/limiter/limiter.h>

static void on_limiter_triggered(const char * topic, void * data, void * priv)
{
    struct limiter_t * limiter = (struct limiter_t *)data;
    /* Emergency stop, axis homing completion, full-bin detection, etc. */
}

static void on_limiter_released(const char * topic, void * data, void * priv)
{
    struct limiter_t * limiter = (struct limiter_t *)data;
    /* Resume operation */
}

psub_subscribe("limiter.triggered", on_limiter_triggered, NULL);
psub_subscribe("limiter.released",  on_limiter_released,  NULL);
```

#### Design Highlights

- **Decoupling**: The driver only publishes events without knowing the number of subscribers, allowing motion control, HMI, logging, and other modules to react simultaneously.
- **Edge-triggered**: Events are published only on state transitions, avoiding redundant notifications. Applications can call `limiter_get_status()` at any time to query the instantaneous state.
- **Portability**: Different concrete drivers (polling, interrupt-driven, I2C IO expanders, etc.) only need to follow the same event topic convention, and no changes are required on the application side.

## Concrete Driver: limiter-gpio-polled

File: `xstar/driver/limiter/limiter-gpio-polled.c`
Config: `CONFIG_DRV_LIMITER_GPIO_POLLED`

A GPIO software-timer polling based limiter driver. Suitable for platforms without external interrupt resources or when unified software-layer debouncing is desired.

### Device Tree Configuration

Add the following entry under `/romdisk/dtree/default.json` using `limiter-gpio-polled` as the node name:

```json
"limiter-gpio-polled@0": {
    "gpio":              123,
    "gpio-config":       -1,
    "active-low":        true,
    "poll-interval-ms":  10
}
```

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `gpio` | int | -1 (required) | Input GPIO number; must pass `gpio_is_valid()` |
| `gpio-config` | int | -1 (no change) | Platform-specific pinmux configuration |
| `active-low` | bool | false | Low-level triggered. When `true`, pull-up is enabled; when `false`, pull-down is enabled |
| `poll-interval-ms` | int | 10 | Polling interval in milliseconds. Determines response latency and debounce window |

During probe the driver performs:

1. Validate the GPIO;
2. Configure the GPIO as input with pull-up/pull-down according to `active-low`;
3. Sample the initial level as the initial state and fill the debounce buffer;
4. Start the periodic timer to begin polling.

### Event-driven Flow

The driver publishes events by periodic sampling + software debouncing + edge detection:

```c
static int limiter_gpio_polled_timer_function(struct timer_t * timer, void * data)
{
    struct limiter_t * limiter = (struct limiter_t *)(data);
    struct limiter_gpio_polled_pdata_t * pdat = limiter->priv;
    int status;

    if(gpio_get_value(pdat->gpio))
        status = pdat->active_low ? 0 : 1;
    else
        status = pdat->active_low ? 1 : 0;

    pdat->buffer[pdat->bufidx] = status;
    pdat->bufidx = (pdat->bufidx + 1) & 0x3;

    if(pdat->status != status)
    {
        if((pdat->buffer[0] == status) && (pdat->buffer[1] == status) &&
           (pdat->buffer[2] == status) && (pdat->buffer[3] == status))
        {
            pdat->status = status;
            if(status)
                psub_publish("limiter.triggered", limiter);
            else
                psub_publish("limiter.released", limiter);
        }
    }
    timer_forward(timer, ms_to_ktime(pdat->interval));
    return 1;
}
```

Workflow:

1. **Polling**: The timer samples the GPIO level every `poll-interval-ms` and computes the logical state based on `active-low`.
2. **Software debounce**: A 4-sample ring buffer (`buffer[4]`). A state change is only accepted when four consecutive samples agree and differ from the previously stable state, effectively filtering mechanical bounce and electrical glitches.
3. **Edge event publishing**: Once the state change is confirmed, `psub_publish()` is called with `limiter.triggered` or `limiter.released`.

> The response latency is approximately `poll-interval-ms × 4`. Trade off between responsiveness (smaller interval) and noise immunity (larger interval) based on the application.

### Active Query (Polling Query)

In addition to events, the upper layer can query the current stable status at any time:

```c
struct limiter_t * lim = search_limiter("limiter-gpio-polled.0");
if(lim && limiter_get_status(lim))
{
    /* Currently triggered */
}
```

`limiter_get_status()` ultimately invokes the driver's `get()` callback, which returns the debounced stable status `pdat->status`. It is not affected by transient bouncing.

### Power Management

- `suspend`: Cancels the polling timer and stops sampling.
- `resume`: Restarts the timer and resumes sampling from the current GPIO level. (Note: level changes that occur during suspend are not replayed; the new stable state after wakeup is taken as the current state.)
