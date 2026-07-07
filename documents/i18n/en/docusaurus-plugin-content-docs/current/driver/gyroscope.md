# Gyroscope (gyroscope)

Gyroscope.

## Device Type

`DEVICE_TYPE_GYROSCOPE`

## Struct

```c
struct gyroscope_t {
    char * name;
    int (*get)(struct gyroscope_t * gyroscope, int * x, int * y, int * z);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_gyroscope(name)` | Find gyroscope by name |
| `register_gyroscope(gyro, drv)` | Register gyroscope |
| `unregister_gyroscope(gyro)` | Unregister gyroscope |
| `gyroscope_get_palstance(gyro, x, y, z)` | Get three-axis angular velocity |

## Description

Gyroscope sensor interface. Returns three-axis angular velocity for orientation tracking. Typically used with an accelerometer for IMU.
