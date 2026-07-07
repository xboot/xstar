# Easing Function (easing)

Easing animation module based on cubic Bezier curves, implementing various acceleration and deceleration effects by defining curve control points.

## Bezier Curve

```c
struct bezier_t {
    float ax, bx, cx;   /* Cubic polynomial coefficients for X component */
    float ay, by, cy;   /* Cubic polynomial coefficients for Y component */
};
```

A standard cubic Bezier curve is defined by 4 control points: P0=(0,0), P1=(x1,y1), P2=(x2,y2), P3=(1,1). `bezier_init` converts control points into polynomial coefficients, `bezier_calc` evaluates via Newton's method + bisection.

```c
void bezier_init(struct bezier_t * b, float x1, float y1, float x2, float y2);
```

Initialize a Bezier curve, `x1/y1` and `x2/y2` are the two middle control points, range [0, 1].

```c
float bezier_calc(struct bezier_t * b, float t);
```

Calculate the curve value at time `t` (0~1). Returns 0 for `t <= 0`, returns 1 for `t >= 1`.

## Easing Animation

```c
struct easing_t {
    struct bezier_t bezier;   /* Bezier curve */
    float start;              /* Start value */
    float stop;               /* End value */
    float duration;           /* Duration (seconds) */
    float acc;                /* Accumulated time */
};
```

### API

```c
void easing_init(struct easing_t * e, float start, float stop, float duration, float x1, float y1, float x2, float y2);
```

Initialize an easing animation. `start`/`stop` define the value range, `duration` is the total duration, `x1/y1/x2/y2` are the Bezier control points.

```c
float easing_calc(struct easing_t * e, float t);
```

Calculate the value at time `t` (seconds).

```c
float easing_step(struct easing_t * e, float dt);
```

Advance by `dt` seconds and return the current value. Internal accumulated time is automatically incremented.

```c
int easing_finished(struct easing_t * e);
```

Whether the animation has finished. Returns 1 when finished.

## Common Curve Parameters

| Effect | x1 | y1 | x2 | y2 |
|------|-----|-----|-----|-----|
| Linear | 0 | 0 | 1 | 1 |
| Ease In | 0.42 | 0 | 1 | 1 |
| Ease Out | 0 | 0 | 0.58 | 1 |
| Ease In Out | 0.42 | 0 | 0.58 | 1 |
| Ease Out Elastic | 0.68 | -0.55 | 0.27 | 1.55 |

## Usage Examples

### Position Animation

```c
struct easing_t e;
easing_init(&e, 0, 300, 0.5, 0.42, 0, 0.58, 1);  /* ease in out, 0→300, 0.5s */

while(!easing_finished(&e))
{
    float pos = easing_step(&e, 0.016);  /* approximately 16ms per frame */
    set_position(pos);
}
```

### Direct Calculation at Specific Time

```c
struct easing_t e;
easing_init(&e, 0, 100, 1.0, 0, 0, 0.58, 1);
float v = easing_calc(&e, 0.5);  /* value at 0.5 seconds */
```

### Using Bezier Curve Independently

```c
struct bezier_t b;
bezier_init(&b, 0.42, 0, 0.58, 1);
float progress = bezier_calc(&b, 0.5);  /* curve value at 50% progress */
```
