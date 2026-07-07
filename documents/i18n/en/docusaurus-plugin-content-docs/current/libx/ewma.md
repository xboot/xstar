# EWMA Filter (ewma)

Exponentially Weighted Moving Average filter, a single-pole low-pass filter that smooths values by blending new samples with the previous average.

Update formula: `avg = weight * value + (1 - weight) * last`

## Data Structure

```c
struct ewma_filter_t {
    float weight;
    float last;
};
```

## API

```c
struct ewma_filter_t * ewma_alloc(float weight);
void ewma_free(struct ewma_filter_t * filter);
float ewma_update(struct ewma_filter_t * filter, float value);
void ewma_clear(struct ewma_filter_t * filter);
```

- `ewma_alloc` — Allocate filter, `weight` range 0~1, higher values track new samples more closely
- `ewma_free` — Free filter
- `ewma_update` — Input new value, returns updated average
- `ewma_clear` — Reset filter state

## Example

### Sensor Data Filtering

```c
struct ewma_filter_t * f = ewma_alloc(0.2f);

while(running)
{
    float sample = read_sensor();
    float filtered = ewma_update(f, sample);
}

ewma_free(f);
```

### Reset Filter

```c
ewma_clear(f);
```
