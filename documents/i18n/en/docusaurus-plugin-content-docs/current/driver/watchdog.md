# Watchdog (watchdog)

Watchdog timer.

## Device Type

`DEVICE_TYPE_WATCHDOG`

## Structure

```c
struct watchdog_t {
    char * name;
    int (*set)(struct watchdog_t * watchdog, int timeout);
    int (*get)(struct watchdog_t * watchdog);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_watchdog(name)` | Find watchdog by name |
| `register_watchdog(wdt, drv)` | Register a watchdog |
| `unregister_watchdog(wdt)` | Unregister a watchdog |
| `watchdog_set_timeout(wdt, timeout)` | Set timeout (seconds) |
| `watchdog_get_timeout(wdt)` | Get remaining time |

## Description

Hardware watchdog timer interface. Timeout value is configurable; setting to 0 stops the watchdog. Used in safety-critical and unattended systems.
