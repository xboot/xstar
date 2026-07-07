# Publish-Subscribe (psub)

A system-level publish-subscribe message bus based on topic-based loosely coupled communication, enabling modules to exchange data without direct references, achieving complete decoupling between producers and consumers.

## Decoupling Benefits

Traditional inter-module communication typically uses direct function calls, creating compile-time dependencies and runtime coupling between modules:

- **Direct Call**: Module A calls module B's function; A must include B's header, and B must initialize before A
- **Publish-Subscribe**: Module A publishes a message to topic `"topic-x"`, module B subscribes to `"topic-x"`; A and B are unaware of each other, no header inclusion needed, initialization order is irrelevant

This decoupling provides the following advantages:

- **Eliminated Dependencies**: No compile-time dependency between publisher and subscriber; each compiles and evolves independently
- **One-to-Many Communication**: A single published event can notify multiple subscribers simultaneously, without the publisher maintaining a subscriber list
- **Dynamic Subscription**: Subscribers can subscribe and unsubscribe at runtime, flexibly responding to system state changes
- **Lifecycle Decoupling**: If there are no subscribers when publishing, the event is silently dropped; once subscribed, new publications are received immediately
- **One-shot Subscription**: The `oneshot` flag supports a "subscribe once, auto-unsubscribe" pattern, useful for initialization wait scenarios

## How It Works

### Internal Structure

`psub` is a system-level wrapper around `libx/ps`, creating a global `psctx_t` context during the `pure_initcall` phase:

```
psctx_t (global context)
  ├── Hash table (topic index)
  │   ├── "wifi.connected" → pstopic_t
  │   │   ├── pscb_t (subscriber 1: cb1, sdat1)
  │   │   └── pscb_t (subscriber 2: cb2, sdat2)
  │   └── "sensor.data" → pstopic_t
  │       └── pscb_t (subscriber 3: cb3, sdat3)
  └── Mutex
```

### Topic Management

- Topics are strings, indexed via string hashing (`shash`)
- The hash table auto-expands: when the number of topics exceeds half the hash table capacity, the capacity doubles
- Topics are created automatically on first subscription and retained when there are no subscribers (not proactively deleted)

### Publish Flow

The processing in `psub_publish()`:

1. Look up the topic in the hash table; if not found, return immediately (event dropped)
2. Splice the subscriber callback list to a local list (outside lock protection)
3. Call each subscriber callback: `cb(pdat, sdat)`
4. After callbacks complete, re-acquire the lock and return non-oneshot subscribers to the list
5. Free memory for oneshot subscribers

This "splice first, then callback" design avoids deadlocks when callbacks publish or subscribe to the same topic again.

### Subscription Deduplication

If a subscription with the exact same `cb` and `sdat` already exists on the same topic, the new subscription request is ignored to prevent duplicate callbacks.

## API

| Function | Description |
|----------|-------------|
| `psub_publish(topic, pdat)` | Publish a message to a topic; `pdat` is the data pointer passed to subscribers |
| `psub_subscribe(topic, cb, sdat, oneshot)` | Subscribe to a topic; `cb` is the callback, `sdat` is subscriber private data, `oneshot` of 1 auto-unsubscribes after one trigger |
| `psub_unsubscribe(topic, cb, sdat)` | Unsubscribe; `cb` of NULL unsubscribes all subscribers on that topic |

Callback prototype: `void cb(void * pdat, void * sdat)`

- `pdat`: Data passed by the publisher
- `sdat`: Private data provided at subscription time

## Usage Examples

### Basic Publish-Subscribe

```c
#include <kernel/core/psub.h>

/* Subscriber */
static void on_wifi_connected(void * pdat, void * sdat)
{
    const char * ssid = (const char *)pdat;
    struct my_app_t * app = (struct my_app_t *)sdat;
    LOG("wifi connected: %s\n", ssid);
    app->wifi_ready = 1;
}

void app_init(void)
{
    struct my_app_t * app = get_app();
    psub_subscribe("wifi.connected", on_wifi_connected, app, 0);
}

/* Publisher (another module, no need to know about app) */
void wifi_driver_notify(void)
{
    psub_publish("wifi.connected", "MyNetwork");
}
```

### One-shot Subscription (Initialization Wait)

```c
/* Wait for a service to be ready, notify only once */
static void on_service_ready(void * pdat, void * sdat)
{
    LOG("service is ready, starting dependent module\n");
    start_my_module();
}

void init_dependent_module(void)
{
    /* oneshot = 1, auto-unsubscribes and frees memory after triggering */
    psub_subscribe("service.ready", on_service_ready, NULL, 1);
}
```

### Unsubscribe

```c
/* Precise unsubscribe */
psub_unsubscribe("sensor.data", my_callback, my_data);

/* Unsubscribe all subscribers on this topic */
psub_unsubscribe("sensor.data", NULL, NULL);
```

### Multiple Subscribers

```c
/* Display module: subscribe to battery level to update UI */
static void on_battery_update(void * pdat, void * sdat)
{
    int level = *(int *)pdat;
    ui_set_battery_level(level);
}
psub_subscribe("battery.level", on_battery_update, NULL, 0);

/* Logging module: subscribe to battery level to log */
static void on_battery_log(void * pdat, void * sdat)
{
    int level = *(int *)pdat;
    LOG("battery: %d%%\n", level);
}
psub_subscribe("battery.level", on_battery_log, NULL, 0);

/* Power driver: publish once, both subscribers receive it */
int level = 85;
psub_publish("battery.level", &level);
```

## Notes

- `psub` automatically initializes a global context during the `pure_initcall()` phase; no manual invocation is needed
- All operations are protected by mutexes, making them thread-safe
- If there are no subscribers when publishing, the event is silently dropped without error
- Callbacks execute synchronously in the publisher's thread; avoid time-consuming operations in callbacks
- Callbacks can safely call `psub_publish()` or `psub_subscribe()` (no deadlock)
- `oneshot` subscriptions automatically free memory after triggering; no manual unsubscribe needed
- In `psub_unsubscribe()`, passing `cb` as NULL unsubscribes all subscribers on that topic
- The underlying implementation is based on `libx/ps`, which supports independent `psctx_t` contexts for isolating different publish-subscribe domains
