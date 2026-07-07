# GPIO Controller (gpio)

GPIO controller.

## Device Type

`DEVICE_TYPE_GPIOCHIP`

## Struct

```c
struct gpiochip_t {
    char * name;
    int base, ngpio;
    int (*set_cfg)(struct gpiochip_t * chip, int offset, int cfg);
    int (*get_cfg)(struct gpiochip_t * chip, int offset);
    int (*set_pull)(struct gpiochip_t * chip, int offset, enum gpio_pull_t pull);
    int (*get_pull)(struct gpiochip_t * chip, int offset);
    int (*set_drv)(struct gpiochip_t * chip, int offset, enum gpio_drv_t drv);
    int (*get_drv)(struct gpiochip_t * chip, int offset);
    int (*set_dir)(struct gpiochip_t * chip, int offset, int dir);
    int (*get_dir)(struct gpiochip_t * chip, int offset);
    int (*set_value)(struct gpiochip_t * chip, int offset, int value);
    int (*get_value)(struct gpiochip_t * chip, int offset);
    int (*to_irq)(struct gpiochip_t * chip, int offset);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_gpiochip(name)` | Find GPIO controller by name |
| `register_gpiochip(chip, drv)` | Register GPIO controller |
| `unregister_gpiochip(chip)` | Unregister GPIO controller |
| `gpio_set/get_value(gpio, value)` | Set/get GPIO value |
| `gpio_direction_output/input(gpio)` | Set output/input mode |
| `gpio_to_irq(gpio)` | Convert GPIO to IRQ number |

## Description

GPIO controller interface. Controls pins using a global numbering space, supports function configuration, pull-up/down, drive strength and direction.
