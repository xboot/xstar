# Spring Animation (spring)

An animation module based on a physical spring model. It simulates spring motion through tension and friction parameters, producing natural bounce effects.

## Principle

Spring motion is controlled by two physical parameters:

- **Tension** — Spring stiffness, larger values cause faster rebounds
- **Friction** — Damping coefficient, larger values cause faster oscillation decay

Motion equation: `acceleration = tension × (target position - current position) - friction × current velocity`

The simulation advances using Euler integration with a fixed step size (1ms), automatically handling overshoot and oscillation.

## Data Structure

```c
struct spring_t {
    float start;       /* Current position */
    float stop;        /* Target position */
    float velocity;    /* Current velocity */
    float tension;     /* Tension */
    float friction;    /* Friction */
    float acc;         /* Accumulated time remainder */
};
```

## API

```c
void spring_init(struct spring_t * s, float start, float stop, float velocity, float tension, float friction);
```

Initialize the spring. `start` is the starting position, `stop` is the target position, `velocity` is the initial velocity, `tension` is the spring stiffness, and `friction` is the damping coefficient.

```c
int spring_step(struct spring_t * s, float dt);
```

Advance by `dt` seconds, internally integrating with 1ms steps. Returns 1 if motion has not ended, returns 0 when the target is reached (both position and velocity are within 0.005 precision). `dt` is capped at 83ms; values exceeding this are truncated.

```c
float spring_position(struct spring_t * s);
```

Get the current position.

```c
float spring_velocity(struct spring_t * s);
```

Get the current velocity.

## Parameter Reference

| Effect | tension | friction |
|--------|---------|----------|
| Fast rebound | 400 | 30 |
| Medium rebound | 200 | 20 |
| Slow rebound | 100 | 15 |
| Almost no oscillation | 200 | 40 |
| Noticeable oscillation | 200 | 10 |

## Usage Examples

### Basic Spring Animation

```c
struct spring_t s;
spring_init(&s, 0, 300, 0, 200, 20);

while(spring_step(&s, 0.016))
{
    float pos = spring_position(&s);
    set_position(pos);
}
float final_pos = spring_position(&s);  /* ≈ 300 */
```

### With Initial Velocity

```c
struct spring_t s;
spring_init(&s, 0, 100, 500, 180, 15);  /* Initial velocity 500 toward target */
```

### Chained Animation

```c
spring_init(&s, current_pos, new_target, 0, 200, 20);
/* Re-initialize each time the target changes */
```
