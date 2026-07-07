# Compass (compass)

Magnetometer/compass.

## Device Type

`DEVICE_TYPE_COMPASS`

## Structure

```c
struct compass_t {
    char * name;
    int ox, oy, oz;
    int (*get)(struct compass_t * compass, int * x, int * y, int * z);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_compass(name)` | Find a compass by name |
| `register_compass(compass, drv)` | Register a compass |
| `unregister_compass(compass)` | Unregister a compass |
| `compass_get_magnetic(compass, x, y, z)` | Get magnetic field values |
| `compass_get_heading(compass, declination)` | Get heading angle |

## Description

Magnetometer sensor interface. Returns three-axis magnetic field readings, supports hard-iron calibration offset configuration.
