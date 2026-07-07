# Light Sensor (light)

Ambient light sensor.

## Device Type

`DEVICE_TYPE_LIGHT`

## Structure

```c
struct light_t {
    char * name;
    int (*get)(struct light_t * light);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_light(name)` | Find light sensor by name |
| `register_light(light, drv)` | Register light sensor |
| `unregister_light(light)` | Unregister light sensor |
| `light_get_illuminance(light)` | Get illuminance |

## Description

Ambient light sensor interface. Returns the current illuminance (lux), used for automatic brightness adjustment.
