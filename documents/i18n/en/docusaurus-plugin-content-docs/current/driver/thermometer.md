# Thermometer (thermometer)

Temperature sensor.

## Device Type

`DEVICE_TYPE_THERMOMETER`

## Structure

```c
struct thermometer_t {
    char * name;
    int (*get)(struct thermometer_t * thermometer);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_thermometer(name)` | Find temperature sensor by name |
| `register_thermometer(t, drv)` | Register a temperature sensor |
| `unregister_thermometer(t)` | Unregister a temperature sensor |
| `thermometer_get(t)` | Get temperature value |

## Description

Temperature sensor interface. Returns temperature readings in millidegrees Celsius, used for system thermal monitoring and environmental sensing.
