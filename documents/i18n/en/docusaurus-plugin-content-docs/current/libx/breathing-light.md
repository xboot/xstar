# Breathing Light (breathing)

Breathing light brightness calculation module, returns the corresponding brightness value based on the current time, simulating the natural fade-in and fade-out effect of breathing.

## Principle

Based on a precomputed 1024-point 8-bit lookup table, the curve follows the logarithmic characteristics of human perception: slow changes in low brightness regions, rapid changes in high brightness regions, producing a visually smooth breathing rhythm. The table values are stored internally in the range `[0, 255]` and scaled back to `[0, 1000]` using integer arithmetic when returned.

The input time automatically wraps around modulo the cycle, no manual period boundary handling is required.

## API

```c
int breathing(int ms);
```

Pass in the current time (milliseconds), returns a brightness value in the range `[0, 1000]`.

- 0 indicates darkest (off)
- 1000 indicates brightest
- The period is 4096ms (approximately 4 seconds), i.e., `ms` cycles every 4096

## Curve Characteristics

- Rising edge approximately 1388ms, falling edge approximately 2672ms, rise/fall ratio approximately 1:1.93
- Simulates natural breathing rhythm: quick inhale, slow exhale
- Smooth transition at the peak, no abrupt changes
- Larger proportion in low brightness range, visually "slowly rising and falling"

### Mathematical Model

The lookup table data is best fitted by an asymmetric sin^p curve, with index i (0 ≤ i < 1024) as the independent variable:

```
y(i) = 1000 × sin^pr(π × i / (2 × Tr))      i ≤ Tr (rising phase)
y(i) = 1000 × sin^pf(π × (1024 - i) / (2 × Tf))   i > Tr (falling phase)
```

| Parameter | Value | Corresponding Time |
|-----------|-------|--------------------|
| Tr (rising half-period) | 349 | 1396ms |
| Tf (falling half-period) | 675 | 2700ms |
| pr (rising exponent) | 4.6 | - |
| pf (falling exponent) | 4.8 | - |

Fitting root mean square error (RMSE) ≈ 12, maximum absolute error ≈ 23.

## Multi-Light Synchronization

`breathing()` is a **stateless pure function** that returns a brightness value purely based on the input time parameter via table lookup, with no global state, counter, or time memory internally. This characteristic allows multiple breathing lights to naturally maintain perfectly synchronized breathing rhythms without any lock, semaphore, or additional coordination mechanism.

### Synchronization Principle

- The function output is determined solely by the input time: the same `ms` input always produces the same brightness output
- All breathing lights only need to reference the **same time base** (such as the system monotonic clock `ktime_get()`) and pass the same `ms` to `breathing(ms)`; the brightness changes of all lights will be identical at any given moment
- Since there is no accumulated error, no phase drift will occur between multiple lights regardless of how long they run

### Phase Offset

By adding a fixed time offset to each light, "flowing" or "wave" effects can be achieved, and the phase differences between lights remain stable:

- `breathing(ms + 1024)`: phase offset of 1/4 period
- `breathing(ms + 2048)`: phase offset of 1/2 period
- `breathing(ms + 3072)`: phase offset of 3/4 period

### Multi-Light Synchronization Example

```c
void multi_led_breathing_loop(void)
{
    ktime_t start = ktime_get();
    while(1)
    {
        int ms = (int)(ktime_to_ms(ktime_get() - start));

        led_set_pwm(0, breathing(ms));
        led_set_pwm(1, breathing(ms));
        led_set_pwm(2, breathing(ms + 1024));
        led_set_pwm(3, breathing(ms + 2048));

        xos_msleep(16);
    }
}
```

In the example above, LED0 and LED1 are perfectly synchronized, while LED2 and LED3 maintain fixed phase differences of 1/4 and 1/2 period from LED0 respectively, producing a stable visual rhythm.

## Usage Examples

### LED Breathing Light

```c
void led_breathing_loop(void)
{
    ktime_t start = ktime_get();
    while(1)
    {
        int ms = (int)(ktime_to_ms(ktime_get() - start));
        int brightness = breathing(ms);
        led_set_pwm(brightness);
        xos_msleep(16);
    }
}
```

### Working with PWM

```c
int duty = breathing(ktime_to_ms(ktime_get()));
int pwm_period = 1000;
pwm_config(pwm_period, duty);
```

The brightness value 0~1000 can be directly mapped to the PWM duty cycle (period set to 1000), achieving brightness control without additional conversion.
