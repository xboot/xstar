# Delayed Callback (delaycall)

A delayed callback mechanism based on the timer system, asynchronously executing a callback function after a specified number of milliseconds, without manual timer lifecycle management.

## How It Works

`delaycall()` internally allocates a `delaycall_pdat_t` structure that encapsulates the timer and callback information:

1. Allocate `delaycall_pdat_t`, storing the user callback function and data
2. Initialize the internal timer with `delaycall_timer_function` as the callback
3. Call `timer_start()` to start the timer with the specified millisecond interval
4. When the timer expires, `delaycall_timer_function` executes the user callback and automatically frees memory

The entire process requires no manual `timer_init()` or `timer_cancel()` calls; the timer is automatically cleaned up after expiration.

## API

| Function | Description |
|----------|-------------|
| `delaycall(ms, func, data)` | Asynchronously execute `func(data)` after `ms` milliseconds |

## Usage Examples

```c
#include <kernel/time/delaycall.h>

static void on_led_off(void * data)
{
    int * gpio = (int *)data;
    led_set(*gpio, 0);  /* Turn off LED after delay */
}

void flash_led(int gpio)
{
    led_set(gpio, 1);           /* Turn on LED */
    delaycall(500, on_led_off, &gpio);  /* Auto-off after 500ms */
}
```

### Deferred Resource Cleanup

```c
static void cleanup_resource(void * data)
{
    struct my_ctx_t * ctx = (struct my_ctx_t *)data;
    xos_mem_free(ctx->buffer);
    xos_mem_free(ctx);
}

void deferred_cleanup(struct my_ctx_t * ctx)
{
    /* Asynchronously cleanup after 100ms, letting current operations finish first */
    delaycall(100, cleanup_resource, ctx);
}
```

## Notes

- Delayed callbacks are based on the timer system (`timer`); precision depends on the clock event device
- Callbacks execute in the timer interrupt context; avoid time-consuming operations
- `delaycall()` internally allocates and frees memory; the caller does not need to manage it
- When `ms` is 0 or `func` is NULL, the function returns immediately without doing anything
- After the callback completes, the internal timer and `delaycall_pdat_t` are automatically freed
