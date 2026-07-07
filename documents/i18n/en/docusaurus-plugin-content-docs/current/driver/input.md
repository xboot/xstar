# Input Device (input)

Input device.

## Device Type

`DEVICE_TYPE_INPUT`

## Struct

```c
struct input_t {
    char * name;
    int (*ioctl)(struct input_t * input, const char * cmd, void * arg);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_input(name)` | Find input device by name |
| `register_input(input, drv)` | Register input device |
| `unregister_input(input)` | Unregister input device |
| `input_ioctl(input, cmd, arg)` | Control input device |

## Description

Generic input device interface (e.g. touchscreen, buttons, keyboard). Uses string ioctl control model. Actual input event generation is handled by the kernel input subsystem.
