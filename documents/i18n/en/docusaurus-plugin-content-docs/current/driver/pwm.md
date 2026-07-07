# PWM (pwm)

Pulse width modulator.

## Device Type

`DEVICE_TYPE_PWM`

## Structure

```c
struct pwm_t {
    char * name;
    int (*config)(struct pwm_t * pwm, int duty, int period, int polarity);
    int (*enable)(struct pwm_t * pwm);
    int (*disable)(struct pwm_t * pwm);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_pwm(name)` | Find PWM by name |
| `register_pwm(pwm, drv)` | Register PWM |
| `unregister_pwm(pwm)` | Unregister PWM |
| `pwm_config(pwm, duty, period, polarity)` | Configure PWM |
| `pwm_enable/disable(pwm)` | Enable/disable PWM |

## Description

Pulse width modulator interface. Configures the duty cycle, period, and polarity of the PWM output. Used for motor control, LED dimming, and tone generation.
