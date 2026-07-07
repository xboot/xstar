# Oximeter (oximeter)

Pulse oximeter.

## Device Type

`DEVICE_TYPE_OXIMETER`

## Structure

```c
struct oximeter_t {
    char * name;
    int (*get)(struct oximeter_t * oximeter, int * sp02, int * rate);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_oximeter(name)` | Find oximeter by name |
| `register_oximeter(oximeter, drv)` | Register oximeter |
| `unregister_oximeter(oximeter)` | Unregister oximeter |
| `oximeter_get(oximeter, sp02, rate)` | Get blood oxygen saturation and heart rate |

## Description

Pulse oximeter sensor interface (e.g., MAX30100/MAX30102). Returns blood oxygen saturation (SpO2) and heart rate via a PPG sensor. Used in wearable health monitoring.
