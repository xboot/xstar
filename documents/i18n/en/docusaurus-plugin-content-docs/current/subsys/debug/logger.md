# Logger (logger)

A circular buffer-based logging system with automatic timestamp insertion. The write path is interrupt-safe and non-blocking, suitable for kernel and driver debug output.

## Configuration

The logging system is controlled via Kconfig options:

| Option | Description |
|--------|-------------|
| `CONFIG_XSTAR_LOG` | Enable the logging system; the `LOG` macro only takes effect when enabled |
| `CONFIG_XSTAR_LOGGER_SIZE` | Circular buffer size in bytes, default 8192 |
| `CONFIG_CMD_DMESG` | Enable the `dmesg` command for viewing and following logs |

## How It Works

The logger follows a producer-consumer model. `logger()` only writes to the buffer and never performs any console output:

- **Write** (`logger_push`): Writes characters one by one to the `head` position, advancing `head`; when the buffer is full, `tail` advances as well, overwriting the oldest data
- **Read** (`logger_dump`): Reads non-destructively from `tail` (or from a cursor held by the caller) and outputs via a callback function; the buffer content is retained after reading
- **Clear** (`logger_clear`): Resets `head` and `tail`, clearing the entire buffer
- **Thread Safety**: All buffer operations are protected by a spinlock (`spinlock`). On bare-metal platforms the spinlock implementation saves/restores interrupts, so the write path can be safely called from interrupt context

Each call to `logger()` automatically prepends a microsecond-level timestamp in the format `[seconds.microseconds]`, then only writes into the circular buffer and returns immediately, without any I/O.

## dmesg Command

Logs are viewed via the shell `dmesg` command:

```
dmesg        # Print all logs retained in the buffer (non-destructive, can be run repeatedly)
dmesg -c     # Print and then clear the buffer
dmesg -w     # Replay existing logs, then keep printing new messages
dmesg -w -c  # Replay existing logs, clear the buffer, then keep printing new messages
```

## API

| Function | Description |
|----------|-------------|
| `LOG(fmt, arg...)` | Logging output macro, same format as `printf`; no-op when `CONFIG_XSTAR_LOG` is not enabled |
| `logger(fmt, ...)` | Low-level logging function, automatically adds a timestamp and writes into the circular buffer |
| `logger_dump(pos, cb, data)` | Reads logs non-destructively and outputs via the callback `cb(buf, len, data)`; when `pos` is `NULL` the entire retained buffer is read, otherwise it reads incrementally from `*pos` and updates the cursor; the cursor is clamped to `tail` if it has been overwritten or cleared |
| `logger_clear()` | Clear the log buffer |

## Usage Example

```c
#include <kernel/core/logger.h>

/* Basic log output */
LOG("value = %d\n", value);
LOG("device %s initialized, rate = %lu\n", name, rate);
```

View in the shell:

```
xstar: /# dmesg
[    1.000123] value = 42
[    1.000456] device uart-pl011 initialized, rate = 115200
```

## Notes

- The `LOG` macro compiles to a no-op when `CONFIG_XSTAR_LOG` is not enabled, with zero runtime overhead
- Timestamps are obtained via `ktime_get()` with microsecond precision
- When the buffer is full, the overwrite strategy retains the newest logs and discards the oldest
- The write path only performs formatting and memory copy, with no blocking I/O; it can be called from interrupt context
- Logs are not streamed to the console in real time; use the `dmesg` command to view them, and `dmesg -w` to continuously follow new messages
- Logging is always active; to remove it completely, disable `CONFIG_XSTAR_LOG` at compile time
