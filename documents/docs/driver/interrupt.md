# 中断控制器 (interrupt)

中断控制器。

## 设备类型

`DEVICE_TYPE_IRQCHIP`

## 结构体

```c
struct irqchip_t {
    char * name;
    int base, nirq;
    void * handler;
    void (*enable)(struct irqchip_t * chip, int offset);
    void (*disable)(struct irqchip_t * chip, int offset);
    void (*settype)(struct irqchip_t * chip, int offset, int type);
    void (*dispatch)(struct irqchip_t * chip);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `register/unregister_irqchip(chip, drv)` | 注册/注销中断控制器 |
| `irq_is_valid(irq)` | 检查中断是否有效 |
| `request_irq(irq, handler, data)` | 请求中断 |
| `free_irq(irq)` | 释放中断 |
| `enable_irq/disable_irq(irq)` | 使能/禁用中断 |

## 说明

中断控制器接口。管理硬件中断线，支持电平/边沿触发类型。支持层次化中断控制器，提供 `request_irq()`/`free_irq()` 用于注册中断处理函数。
