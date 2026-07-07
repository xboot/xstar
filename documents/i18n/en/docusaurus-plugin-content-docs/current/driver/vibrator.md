# Vibrator (vibrator)

Vibration motor.

## Device Type

`DEVICE_TYPE_VIBRATOR`

## Structure

```c
struct vibrator_t {
    char * name;
    void (*set)(struct vibrator_t * vib, int state);
    int (*get)(struct vibrator_t * vib);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_vibrator(name)` | Find vibrator by name |
| `search_first_vibrator()` | Find the first vibrator |
| `register_vibrator(vib, drv)` | Register a vibrator |
| `unregister_vibrator(vib)` | Unregister a vibrator |
| `vibrator_set_state(vib, state)` | Set vibrator state (0 or 1) |
| `vibrator_get_state(vib)` | Get vibrator state |

## Async API

| Function | Description |
|------|------|
| `vibrator_async_ctx_alloc(name)` | Allocate an async context |
| `vibrator_async_ctx_free(ctx)` | Free an async context |
| `vibrator_async_ctx_clear(ctx)` | Clear pending tasks in async context |
| `vibrator_async_vibrate(ctx, state, millisecond)` | Vibrate for specified milliseconds |
| `vibrator_async_play(ctx, morse)` | Play Morse code pattern |

## Description

Vibration motor control interface. Supports on/off control and async queued vibration patterns. The async API is based on `thworker_t` thread work queue.

`vibrator_async_play()` accepts a Morse code string for pattern output (`.` = 100ms short, `-` = 300ms long).

## Drivers

### vibrator-gpio

GPIO-based vibrator driver.

Required properties:
- `gpio`: vibrator attached pin

Optional properties:
- `gpio-config`: gpio pin configuration
- `active-low`: low level for active vibrator

Example:
```json
"vibrator-gpio": {
    "gpio": 0,
    "gpio-config": -1,
    "active-low": true
}
```

### vibrator-pwm

PWM-based vibrator driver.

Required properties:
- `pwm-name`: vibrator attached pwm

Optional properties:
- `pwm-period-ns`: pwm period in ns
- `pwm-polarity`: pwm polarity

Example:
```json
"vibrator-pwm": {
    "pwm-name": "pwm.0",
    "pwm-period-ns": 2272727,
    "pwm-polarity": true
}
```
