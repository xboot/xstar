# Accelerometer (gmeter)

Accelerometer.

## Device Type

`DEVICE_TYPE_GMETER`

## Struct

```c
struct gmeter_t {
    char * name;
    int (*get)(struct gmeter_t * gmeter, int * x, int * y, int * z);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_gmeter(name)` | Find accelerometer by name |
| `register_gmeter(gmeter, drv)` | Register accelerometer |
| `unregister_gmeter(gmeter)` | Unregister accelerometer |
| `gmeter_get_acceleration(gmeter, x, y, z)` | Get three-axis acceleration values |

## Description

Accelerometer sensor interface. Returns three-axis acceleration values for motion detection and orientation sensing.
