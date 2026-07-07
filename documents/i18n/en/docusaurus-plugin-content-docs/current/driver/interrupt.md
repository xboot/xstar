# Interrupt Controller (interrupt)

Interrupt controller.

## Device Type

`DEVICE_TYPE_IRQCHIP`

## Struct

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

## Key API

| Function | Description |
|------|------|
| `register/unregister_irqchip(chip, drv)` | Register/unregister interrupt controller |
| `irq_is_valid(irq)` | Check if IRQ is valid |
| `request_irq(irq, handler, data)` | Request IRQ |
| `free_irq(irq)` | Free IRQ |
| `enable_irq/disable_irq(irq)` | Enable/disable IRQ |

## Description

Interrupt controller interface. Manages hardware interrupt lines, supports level/edge trigger types. Supports hierarchical interrupt controllers, provides `request_irq()`/`free_irq()` for registering interrupt handlers.
