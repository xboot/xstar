# Pressure Sensor (pressure)

Barometric pressure sensor.

## Device Type

`DEVICE_TYPE_PRESSURE`

## Structure

```c
struct pressure_t {
    char * name;
    int (*get)(struct pressure_t * pressure, int * pa, int * temp);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_pressure(name)` | Find pressure sensor by name |
| `register_pressure(pressure, drv)` | Register pressure sensor |
| `unregister_pressure(pressure)` | Unregister pressure sensor |
| `pressure_get(pressure, pa, temp)` | Get pressure and temperature |

## Description

Barometric pressure and temperature sensor interface (e.g., BMP180/BMP280). Used for altitude estimation and weather monitoring.
