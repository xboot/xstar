# DC Motor (motor)

DC motor.

## Device Type

`DEVICE_TYPE_MOTOR`

## Structure

```c
struct motor_t {
    char * name;
    int (*enable)(struct motor_t * motor);
    int (*disable)(struct motor_t * motor);
    int (*set)(struct motor_t * motor, int speed);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_motor(name)` | Find motor by name |
| `register_motor(motor, drv)` | Register motor |
| `unregister_motor(motor)` | Unregister motor |
| `motor_enable/disable(motor)` | Enable/disable motor |
| `motor_set_speed(motor, speed)` | Set rotational speed |
| `motor_async_play(ctx, ...)` | Asynchronous timed run |

## Description

DC motor control interface. Enable/disable the motor and set rotational speed. The async context supports non-blocking timed operations.
