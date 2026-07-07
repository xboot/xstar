# Timer (timer)

A high-precision timer based on a red-black tree, using nanosecond-level `ktime_t` timestamps, driven by clock events (clockevent) to trigger callbacks, supporting both periodic and one-shot timing.

## Data Structures

### Timer

```c
struct timer_t {
    struct rb_node node;                          /* Red-black tree node */
    struct timer_base_t * base;                   /* Owning timer base */
    enum timer_state_t state;                     /* Timer state */
    ktime_t expires;                              /* Expiration time */
    void * data;                                  /* User data */
    int (*function)(struct timer_t *, void *);    /* Callback function, non-zero return means restart */
};
```

### Timer Base

```c
struct timer_base_t {
    struct rb_root head;       /* Red-black tree root */
    struct timer_t * next;     /* Earliest-expiring timer */
    struct clockevent_t * ce;  /* Bound clock event device */
    struct mutex_t lock;       /* Mutex protecting red-black tree operations */
};
```

### State Machine

```c
enum timer_state_t {
    TIMER_STATE_INACTIVE = 0,  /* Inactive, not in the red-black tree */
    TIMER_STATE_ENQUEUED = 1,  /* Enqueued, waiting to expire */
    TIMER_STATE_CALLBACK = 2,  /* Currently executing callback */
};
```

## How It Works

### Red-Black Tree Management

All active timers are inserted into the red-black tree keyed by expiration time (`expires`), with the earliest-expiring timer at the leftmost node. `base->next` caches the earliest-expiring timer to avoid traversing the tree each time.

### Clock Event Driven

The timer system binds to a clock event device (clockevent):

1. `timer_bind_clockevent()` binds the clock event device and registers an event handler
2. When a timer is enqueued, the earliest expiration time is programmed into the clock event device
3. The clock event interrupt triggers `timer_event_handler()`
4. The handler dequeues all expired timers from the red-black tree and executes their callbacks
5. When a callback returns a non-zero value, the timer is re-enqueued (achieving periodic timing)
6. After processing, the next earliest expiration time is programmed into the clock event device

### Thread Safety

All red-black tree operations are protected by a mutex (`base->lock`), supporting safe use in multi-threaded environments. Callback functions are executed while holding the lock; calling `timer_start()` or `timer_cancel()` on the same timer within a callback is not allowed.

## API

| Function | Description |
|----------|-------------|
| `timer_init(timer, function, data)` | Initialize the timer, set the callback function and user data |
| `timer_start(timer, interval)` | Start the timer, `interval` is a relative time interval (`ktime_t`), callback fires when expired |
| `timer_forward(timer, interval)` | Update the timer's expiration time, only used in callbacks to set the next expiration |
| `timer_cancel(timer)` | Cancel the timer, remove it from the red-black tree |
| `timer_bind_clockevent(ce)` | Bind a clock event device, called during system initialization |

## Usage Examples

### One-shot Timer

```c
#include <kernel/time/timer.h>

static int on_timeout(struct timer_t * timer, void * data)
{
    LOG("timeout!\n");
    return 0;  /* Return 0, do not restart */
}

void demo(void)
{
    struct timer_t timer;
    timer_init(&timer, on_timeout, NULL);
    timer_start(&timer, ms_to_ktime(500));  /* Fire after 500ms */

    /* ... */

    timer_cancel(&timer);  /* Cancel when no longer needed */
}
```

### Periodic Timer

```c
static int on_periodic(struct timer_t * timer, void * data)
{
    int * count = (int *)data;
    LOG("tick: %d\n", (*count)++);
    return 1;  /* Return non-zero, auto-restart */
}

void demo(void)
{
    static struct timer_t timer;
    static int count = 0;

    timer_init(&timer, on_periodic, &count);
    timer_start(&timer, ms_to_ktime(1000));  /* Fire every 1s */
}
```

### Adjusting Period in Callback

```c
static int on_variable(struct timer_t * timer, void * data)
{
    LOG("fired\n");
    timer_forward(timer, ms_to_ktime(200));  /* Set next fire to 200ms */
    return 1;  /* Return 1, re-enqueue with the forwarded time */
}
```

## Notes

- Timer precision depends on the bound clock event device; timestamps use nanosecond-level `ktime_t`
- `timer_start()` can be called repeatedly; it cancels the existing timer before re-enqueuing
- `timer_forward()` only modifies the `expires` field and does not re-enqueue; it must be used with a callback return value of 1
- When a callback returns 1, the timer is re-enqueued with the current `expires` value; if `timer_forward()` was called in the callback, the updated time is used
- Callbacks are executed while holding the lock and should be as short as possible to avoid affecting the precision of other timers
- The timer system is initialized early during system startup via `pure_initcall()`
