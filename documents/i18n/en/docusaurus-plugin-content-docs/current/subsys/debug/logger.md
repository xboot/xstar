# Logger (logger)

A circular buffer-based logging system with automatic timestamp insertion and runtime enable/disable control, suitable for kernel and driver debug output.

## Configuration

The logging system is controlled via Kconfig options:

| Option | Description |
|--------|-------------|
| `CONFIG_XSTAR_LOG` | Enable the logging system; the `LOG` macro only takes effect when enabled |
| `CONFIG_XSTAR_LOGGER_SIZE` | Circular buffer size in bytes, default 8192 |

## How It Works

The logger maintains an internal circular buffer (`buffer`) managed by two pointers, `head` and `tail`:

- **Write** (`logger_push`): Writes characters one by one to the `head` position, advancing `head`; when the buffer is full, `tail` advances as well, overwriting the oldest data
- **Flush** (`logger_pop`): Reads from `tail` one by one and outputs via `xos_stdio_write` to stdout, until it catches up with `head`
- **Thread Safety**: All buffer operations are protected by a spinlock (`spinlock`), supporting multi-threaded/multi-core environments

Each call to `logger()` automatically prepends a microsecond-level timestamp in the format `[seconds.microseconds]`, then formats the user input and immediately flushes the output.

## API

| Function | Description |
|----------|-------------|
| `LOG(fmt, arg...)` | Logging output macro, same format as `printf`; no-op when `CONFIG_XSTAR_LOG` is not enabled |
| `logger(fmt, ...)` | Low-level logging function, automatically adds timestamp and outputs |
| `logger_enable()` | Enable log output |
| `logger_disable()` | Disable log output |
| `logger_status()` | Query log status, returns 1 if enabled, 0 if disabled |

## Usage Example

```c
#include <kernel/core/logger.h>

/* Basic log output */
LOG("value = %d\n", value);
LOG("device %s initialized, rate = %lu\n", name, rate);

/* Runtime control */
logger_disable();  /* Temporarily disable logging */
/* ... sensitive operations ... */
logger_enable();   /* Restore logging */

/* Query status */
if(logger_status())
    LOG("logger is active\n");
```

Example output:

```
[    1.000123] value = 42
[    1.000456] device uart-pl011 initialized, rate = 115200
```

## Notes

- The `LOG` macro compiles to a no-op when `CONFIG_XSTAR_LOG` is not enabled, with zero runtime overhead
- Timestamps are obtained via `ktime_get()` with microsecond precision
- When the buffer is full, the overwrite strategy retains the newest logs and discards the oldest
- `logger()` returns immediately in the `disabled` state without performing any formatting or writing
- Log output is implemented via `xos_stdio_write`; the actual output target depends on the platform XOS implementation
