# Proximity Sensor (proximity)

Proximity sensor.

## Device Type

`DEVICE_TYPE_PROXIMITY`

## Structure

```c
struct proximity_t {
    char * name;
    int (*get)(struct proximity_t * proximity);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_proximity(name)` | Find proximity sensor by name |
| `register_proximity(prox, drv)` | Register proximity sensor |
| `unregister_proximity(prox)` | Unregister proximity sensor |
| `proximity_get_distance(prox)` | Get distance |

## Description

Proximity sensor interface. Returns distance readings from infrared or ultrasonic sensors, used for object detection and gesture sensing.
