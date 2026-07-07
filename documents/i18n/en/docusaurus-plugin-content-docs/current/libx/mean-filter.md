# Mean Filter (mean)

Sliding window mean filter that performs moving average on a data stream to smooth noise. O(1) update complexity.

## Principle

Maintains a fixed-length ring buffer. Each time a new value is input, the overwritten old value is subtracted and the new value is added, achieving O(1) mean computation without traversing the entire window.

```
Window [3, 5, 7], sum=15, mean=5
Input 9 → Window [5, 7, 9], sum=21, mean=7
```

## Data Structure

```c
struct mean_filter_t {
    int * buffer;    /* Ring buffer */
    int length;      /* Window length */
    int index;       /* Current write position */
    int count;       /* Current number of valid samples */
    int sum;         /* Cumulative sum within the window */
};
```

## API

```c
struct mean_filter_t * mean_alloc(int length);
```

Allocate a filter, `length` is the window length.

```c
void mean_free(struct mean_filter_t * filter);
```

Free the filter.

```c
int mean_update(struct mean_filter_t * filter, int value);
```

Input a new value, returns the current window mean (integer division). When the window is not yet full, computes using the actual number of samples.

```c
void mean_clear(struct mean_filter_t * filter);
```

Clear the buffer and reset the state.

## Usage Example

```c
struct mean_filter_t * f = mean_alloc(5);

int v;
while(read_sensor(&v))
{
    int avg = mean_update(f, v);
    printf("raw=%d, avg=%d\n", v, avg);
}

mean_free(f);
```
