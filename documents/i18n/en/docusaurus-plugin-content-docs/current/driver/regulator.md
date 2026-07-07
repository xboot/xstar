# Voltage Regulator (regulator)

Voltage regulator.

## Device Type

`DEVICE_TYPE_REGULATOR`

## Structure

```c
struct regulator_t {
    char * name;
    int count;
    struct regulator_t * (*set_parent)(struct regulator_t * reg, struct regulator_t * parent);
    struct regulator_t * (*get_parent)(struct regulator_t * reg);
    int (*set_enable)(struct regulator_t * reg, int enable);
    int (*get_enable)(struct regulator_t * reg);
    int (*set_voltage)(struct regulator_t * reg, int voltage);
    int (*get_voltage)(struct regulator_t * reg);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_regulator(name)` | Find regulator by name |
| `register_regulator(reg, drv)` | Register a regulator |
| `unregister_regulator(reg)` | Unregister a regulator |
| `regulator_enable/disable(name)` | Enable/disable |
| `regulator_set/get_voltage(name, volt)` | Set/get voltage |

## Description

Voltage regulator interface (e.g. PMIC, LDO, DC-DC). Controls enable/disable and voltage setting of power rails, supports cascaded regulator trees and reference count tracking.
