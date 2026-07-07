# Console (console)

Console serial port.

## Device Type

`DEVICE_TYPE_CONSOLE`

## Structure

```c
struct console_t {
    char * name;
    ssize_t (*read)(struct console_t * console, uint8_t * buf, size_t count);
    ssize_t (*write)(struct console_t * console, uint8_t * buf, size_t count);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_console(name)` | Find a console by name |
| `register_console(console, drv)` | Register a console |
| `unregister_console(console)` | Unregister a console |
| `console_read(console, buf, count)` | Read data |
| `console_write(console, buf, count)` | Write data |

## Description

Character-oriented console I/O device (serial terminal), used for interactive Shell and log output.
