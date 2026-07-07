# Touchscreen Filter (tsfilter)

A touchscreen coordinate filter that applies median debouncing + mean smoothing + affine calibration to raw touch data, outputting stable screen coordinates.

## Filter Pipeline

```
Raw coordinates (x, y)
    ↓
Median filter — Remove impulse noise (glitches)
    ↓
Mean filter — Smooth jitter
    ↓
Affine calibration (cal) — Map raw coordinates to screen coordinates
    ↓
Output coordinates (x, y)
```

## Data Structure

```c
struct tsfilter_t {
    struct median_filter_t * mx, * my;   /* X/Y median filters */
    struct mean_filter_t * nx, * ny;     /* X/Y mean filters */
    int cal[7];                          /* Affine calibration parameters */
};
```

## Calibration Parameters

`cal[7]` defines the affine transformation from raw coordinates to screen coordinates:

```
screen_x = (cal[2] + cal[0] * raw_x + cal[1] * raw_y) / cal[6]
screen_y = (cal[5] + cal[3] * raw_x + cal[4] * raw_y) / cal[6]
```

Default values correspond to an identity transform (no rotation, no offset):

| Index | Default | Meaning |
|-------|---------|---------|
| cal[0] | 1 | X scale factor |
| cal[1] | 0 | Y→X rotation factor |
| cal[2] | 0 | X offset |
| cal[3] | 0 | X→Y rotation factor |
| cal[4] | 1 | Y scale factor |
| cal[5] | 0 | Y offset |
| cal[6] | 1 | Overall divisor |

Actual parameters can be computed via a five-point calibration algorithm to eliminate rotation, scaling, and offset of the touchscreen.

## API

```c
struct tsfilter_t * tsfilter_alloc(int ml, int nl);
```

Allocate a filter. `ml` is the median filter window length, `nl` is the mean filter window length.

```c
void tsfilter_free(struct tsfilter_t * filter);
```

Free the filter and its internal sub-filters.

```c
void tsfilter_setcal(struct tsfilter_t * filter, int * cal);
```

Set calibration parameters. `cal` is a 7-element array.

```c
void tsfilter_update(struct tsfilter_t * filter, int * x, int * y);
```

Input raw coordinates, update in-place to filtered and calibrated screen coordinates. `x` and `y` are input/output parameters.

```c
void tsfilter_clear(struct tsfilter_t * filter);
```

Clear all sub-filter states.

## Usage Examples

### Basic Usage

```c
struct tsfilter_t * f = tsfilter_alloc(5, 4);

int x = raw_x, y = raw_y;
tsfilter_update(f, &x, &y);
/* x, y are filtered and calibrated screen coordinates */

tsfilter_free(f);
```

### With Calibration Parameters

```c
struct tsfilter_t * f = tsfilter_alloc(5, 4);

int cal[7] = { 168012, -57, -137136144, -344, 8674, -2840431, 65536 };
tsfilter_setcal(f, cal);

int x = raw_x, y = raw_y;
tsfilter_update(f, &x, &y);

tsfilter_free(f);
```

### Integration in Touch Driver

```c
void ts_event_handler(int raw_x, int raw_y)
{
    int x = raw_x, y = raw_y;
    tsfilter_update(&filter, &x, &y);
    input_report_abs(INPUT_X, x);
    input_report_abs(INPUT_Y, y);
}
```
