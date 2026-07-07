# Median Filter (median)

Sliding window median filter that computes the median of a data stream, effectively filtering impulse noise (spikes). O(n) update complexity.

## Principle

Maintains a fixed-length ring buffer and a sorted index array. When a new value is input, the index array is updated through insertion-sort-like movement, and the median is directly taken from the middle element after sorting.

Compared to the mean filter, the median filter can effectively eliminate spike noise without blurring signal edges:

```
Input:  [100, 102, 999, 98, 101]   ← 999 is a spike
Mean:   280 (pulled up by spike)
Median: 101 (spike ignored)
```

## Data Structure

```c
struct median_filter_t {
    int * buffer;    /* Ring buffer */
    int * index;     /* Sorted index array, index[i] points to the position of the i-th smallest element */
    int length;      /* Window length */
    int position;    /* Current write position */
    int count;       /* Current number of valid samples */
};
```

## API

```c
struct median_filter_t * median_alloc(int length);
```

Allocate a filter, `length` is the window length.

```c
void median_free(struct median_filter_t * filter);
```

Free the filter.

```c
int median_update(struct median_filter_t * filter, int value);
```

Input a new value, returns the current window median. When the window is not yet full, computes using the actual number of samples.

```c
void median_clear(struct median_filter_t * filter);
```

Clear the buffer and reset the state.

## Usage Examples

### Sensor de-spiking

```c
struct median_filter_t * f = median_alloc(5);

int v;
while(read_sensor(&v))
{
    int filtered = median_update(f, v);
    printf("raw=%d, median=%d\n", v, filtered);
}

median_free(f);
```

### Mean + median cascade

```c
struct median_filter_t * mf = median_alloc(5);
struct mean_filter_t * af = mean_alloc(3);

int v;
while(read_sensor(&v))
{
    int med = median_update(mf, v);
    int avg = mean_update(af, med);
    /* avg is the result of median filtering followed by mean smoothing */
}

median_free(mf);
mean_free(af);
```
