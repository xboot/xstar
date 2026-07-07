# GPIO控制器 (gpio)

GPIO 控制器。

## 设备类型

`DEVICE_TYPE_GPIOCHIP`

## 结构体

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

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_gpiochip(name)` | 按名称查找 GPIO 控制器 |
| `register_gpiochip(chip, drv)` | 注册 GPIO 控制器 |
| `unregister_gpiochip(chip)` | 注销 GPIO 控制器 |
| `gpio_set/get_value(gpio, value)` | 设置/获取 GPIO 值 |
| `gpio_direction_output/input(gpio)` | 设置输出/输入模式 |
| `gpio_to_irq(gpio)` | GPIO 转中断号 |

## 说明

GPIO 控制器接口。使用全局编号空间控制引脚，支持功能配置、上拉/下拉、驱动强度和方向。
