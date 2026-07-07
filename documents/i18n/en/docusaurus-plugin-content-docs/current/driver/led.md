# LED (led)

LED.

## Device Type

`DEVICE_TYPE_LED`

## Struct

```c
struct led_t {
    char * name;
    int (*set)(struct led_t * led, int brightness);
    int (*get)(struct led_t * led);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_led(name)` | Find LED by name |
| `register_led(led, drv)` | Register LED |
| `unregister_led(led)` | Unregister LED |
| `led_set_brightness(led, brightness)` | Set brightness |
| `led_get_brightness(led)` | Get brightness |

## Description

Simple LED brightness control interface. Brightness ranges from 0 (off) to 1000 (full on). Used for indicator lights, status LEDs and backlight control.
