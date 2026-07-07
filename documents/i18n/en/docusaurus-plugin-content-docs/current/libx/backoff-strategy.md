# Backoff Strategy (backoff)

Exponential backoff with random jitter retry delay calculation module, used for scenarios such as network requests and device reconnection that require progressively increasing retry intervals.

## Principle

Each call to `backoff_next()` first computes a random value within `[0, delay]` as the return value, then updates `delay` (exponential growth). Random jitter prevents multiple clients from retrying simultaneously and causing a "thundering herd".

## Data Structure

```c
struct backoff_t {
    int max_delay;   /* Maximum delay cap (milliseconds) */
    int max_retry;   /* Maximum retry count, <= 0 means unlimited */
    int delay;       /* Current delay base value (milliseconds) */
    int retry;       /* Number of retries performed */
};
```

## API

```c
void backoff_init(struct backoff_t * ctx, int delay, int max_delay, int max_retry);
```

Initialize the backoff context. `delay` is the initial delay, `max_delay` is the delay cap, `max_retry` is the maximum retry count (`<=0` means unlimited). `delay` is clamped to the range `[0, max_delay]`.

```c
int backoff_next(struct backoff_t * ctx);
```

Get the wait time (milliseconds) for the next retry. Computes the return value based on the current `delay`, then updates `delay`. Returns -1 when the maximum retry count is reached.

## Growth Strategy

After each call to `backoff_next()`, `delay` is updated according to the following rules:

| Condition | Update Rule |
|-----------|-------------|
| `delay < max_delay / 2` | `delay += max(delay, 1)` (equivalent to doubling when delay >= 1) |
| `delay >= max_delay / 2` | `delay = max_delay` (capped) |

The threshold `max_delay / 2` is computed via integer right shift (`max_delay >> 1`). For example, when `max_delay = 1000`, the threshold is 500.

## Evolution Example

Taking `backoff_init(&bo, 10, 1000, 10)` as an example, the evolution of `delay`:

| Call # | Current delay | Return range | Updated delay | Notes |
|--------|--------------|-------------|---------------|-------|
| 1 | 10 | [0, 10] | 20 | 10 < 500 → 10+10=20 |
| 2 | 20 | [0, 20] | 40 | 20 < 500 → 20+20=40 |
| 3 | 40 | [0, 40] | 80 | 40 < 500 → 40+40=80 |
| 4 | 80 | [0, 80] | 160 | 80 < 500 → 80+80=160 |
| 5 | 160 | [0, 160] | 320 | 160 < 500 → 160+160=320 |
| 6 | 320 | [0, 320] | 640 | 320 < 500 → 320+320=640 |
| 7 | 640 | [0, 640] | 1000 | 640 >= 500 → capped |
| 8 | 1000 | [0, 1000] | 1000 | Already capped |
| 9 | 1000 | [0, 1000] | 1000 | Already capped |
| 10 | 1000 | [0, 1000] | 1000 | Already capped |
| 11 | - | Returns -1 | - | Retries exhausted |

Actual runtime output (return values are random due to jitter):

```
[0] next = 1
[1] next = 2
[2] next = 14
[3] next = 30
[4] next = 16
[5] next = 182
[6] next = 265
[7] next = 250
[8] next = 352
[9] next = 308
[10] next = -1
```

## Usage Examples

### Network Reconnection

```c
struct backoff_t bo;
backoff_init(&bo, 100, 30000, 10);

int wait;
while((wait = backoff_next(&bo)) >= 0)
{
    if(try_connect() == SUCCESS)
        break;
    xos_msleep(wait);
}
if(wait < 0)
    /* retries exhausted */
```

### Unlimited Retries

```c
struct backoff_t bo;
backoff_init(&bo, 50, 5000, 0);  /* max_retry<=0, unlimited */

int wait;
while(1)
{
    if(try_connect() == SUCCESS)
        break;
    wait = backoff_next(&bo);
    xos_msleep(wait);
}
```
