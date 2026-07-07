# Hygrometer (hygrometer)

Temperature and humidity sensor.

## Device Type

`DEVICE_TYPE_HYGROMETER`

## Struct

```c
struct hygrometer_t {
    char * name;
    int (*get)(struct hygrometer_t * hygrometer, int * temp, int * humi);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_hygrometer(name)` | Find temperature/humidity sensor by name |
| `register_hygrometer(hygro, drv)` | Register temperature/humidity sensor |
| `unregister_hygrometer(hygro)` | Unregister temperature/humidity sensor |
| `hygrometer_get(hygro, temp, humi)` | Get temperature and humidity |

## Description

Temperature and humidity sensor interface. Returns relative humidity and temperature readings for environmental monitoring.
