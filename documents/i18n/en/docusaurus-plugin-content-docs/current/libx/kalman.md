# Kalman Filter (kalman)

Single-variable linear Kalman filter implementing predict-update cycle with configurable state transition, observation, process noise, and measurement noise parameters.

## Data Structure

```c
struct kalman_filter_t {
    float a, h;
    float q, r;
    float x, p;
    float k, a2, h2;
};
```

## API

```c
struct kalman_filter_t * kalman_alloc(float a, float h, float q, float r);
void kalman_free(struct kalman_filter_t * filter);
float kalman_update(struct kalman_filter_t * filter, float value);
void kalman_clear(struct kalman_filter_t * filter);
```

- `kalman_alloc` — Allocate a Kalman filter. `a` is the state transition factor, `h` is the observation factor, `q` is the process noise variance, `r` is the measurement noise variance. Returns NULL on failure
- `kalman_free` — Free the filter
- `kalman_update` — Feed a new measurement `value`, perform predict-update cycle, return the filtered estimate
- `kalman_clear` — Reset the filter state (sets estimate to NaN, error covariance to q, gain to 1)

## Example

```c
struct kalman_filter_t * kf = kalman_alloc(1.0f, 1.0f, 0.01f, 0.1f);
if(!kf)
    return;

float measurements[] = { 10.1f, 9.8f, 10.3f, 9.9f, 10.0f };
for(int i = 0; i < 5; i++)
{
    float estimate = kalman_update(kf, measurements[i]);
}

kalman_free(kf);
```
