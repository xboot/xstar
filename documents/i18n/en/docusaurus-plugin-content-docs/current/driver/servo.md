# Servo (servo)

Servo.

## Device Type

`DEVICE_TYPE_SERVO`

## Structure

```c
struct servo_t {
    char * name;
    int (*enable)(struct servo_t * servo);
    int (*disable)(struct servo_t * servo);
    int (*range)(struct servo_t * servo);
    int (*get)(struct servo_t * servo);
    int (*set)(struct servo_t * servo, int angle);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_servo(name)` | Find servo by name |
| `register_servo(servo, drv)` | Register a servo |
| `unregister_servo(servo)` | Unregister a servo |
| `servo_enable/disable(servo)` | Enable/disable servo |
| `servo_set/get_angle(servo, angle)` | Set/get angle |

## Description

Servo control interface. Controls servo position within an angular range, used in robotics, camera gimbals, and automation.
