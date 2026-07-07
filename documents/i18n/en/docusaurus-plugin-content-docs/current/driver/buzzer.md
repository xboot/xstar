# Buzzer (buzzer)

Buzzer control interface, supports frequency control and async beep patterns.

## Device Type

`DEVICE_TYPE_BUZZER`

## Structure

```c
struct buzzer_t {
    char * name;
    void (*set)(struct buzzer_t * buzzer, int frequency);
    int (*get)(struct buzzer_t * buzzer);
    void * priv;
};
```

## Core API

| Function | Description |
|------|------|
| `search_buzzer(name)` | Find a buzzer by name |
| `search_first_buzzer()` | Find the first buzzer |
| `register_buzzer(buzzer, drv)` | Register a buzzer |
| `unregister_buzzer(buzzer)` | Unregister a buzzer |
| `buzzer_set_frequency(buzzer, frequency)` | Set buzzer frequency (0 to disable) |
| `buzzer_get_frequency(buzzer)` | Get current frequency |

## Async API

Async buzzer control based on `thworker_t` thread work queue.

```c
struct buzzer_async_ctx_t {
    struct buzzer_t * buzzer;
    struct thworker_t * worker;
};
```

| Function | Description |
|------|------|
| `buzzer_async_ctx_alloc(name)` | Allocate an async context, uses first buzzer if name is NULL |
| `buzzer_async_ctx_free(ctx)` | Free an async context |
| `buzzer_async_ctx_clear(ctx)` | Clear pending tasks and stop buzzer |
| `buzzer_async_beep(ctx, frequency, millisecond)` | Async beep at specified frequency for specified milliseconds |
| `buzzer_async_play(ctx, rtttl)` | Play RTTTL melody asynchronously |

### RTTTL Format

RTTTL (RingTone Text Transfer Language) is the Nokia ringtone format. Example:

```
Simpsons:d=4,o=5,b=160:32p,c.6,e6,f#6,8a6,g.6,e6,c6,8a,8f#,8f#,8f#,2g
```

Format: `<name>:<control>:<tones>`. Control fields: `d`=default duration, `o`=default octave, `b`=BPM.

## Drivers

### buzzer-gpio

GPIO-based buzzer driver.

Required properties:
- `gpio`: buzzer attached pin

Optional properties:
- `gpio-config`: gpio pin configuration
- `active-low`: low level for active buzzer

Example:
```json
"buzzer-gpio": {
    "gpio": 0,
    "gpio-config": -1,
    "active-low": true
}
```

### buzzer-pwm

PWM-based buzzer driver.

Required properties:
- `pwm-name`: buzzer attached pwm

Optional properties:
- `pwm-polarity`: pwm polarity

Example:
```json
"buzzer-pwm": {
    "pwm-name": "pwm.0",
    "pwm-polarity": false
}
```
