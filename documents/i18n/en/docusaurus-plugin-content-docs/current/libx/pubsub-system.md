# Publish-Subscribe System (psub)

XSTAR's Publish-Subscribe mechanism enables loosely coupled communication between modules. Publishers send messages on a topic, and callbacks subscribed to that topic are automatically invoked. Publishers and subscribers are unaware of each other.

## Architecture

```
Publisher                   Subscriber
psub_publish("net/status", data)  →  cb1(data, sdat1)
                                   →  cb2(data, sdat2)
                                   →  ... (all callbacks subscribed to this topic)
```

A global context is created at system startup via `pure_initcall`, providing `psub_*` global convenience interfaces. The underlying `ps_*` implementation supports creating independent context instances.

## Two-layer Interface

### Global Interface (psub)

Uses the system-wide global publish-subscribe context, simple and direct, suitable for most scenarios.

```c
void psub_publish(const char * topic, void * pdat);
void psub_subscribe(const char * topic, void (*cb)(void *, void *), void * sdat, int oneshot);
void psub_unsubscribe(const char * topic, void (*cb)(void *, void *), void * sdat);
```

### Instance Interface (ps)

Can create independent publish-subscribe contexts, suitable for scenarios requiring isolation (e.g., different subsystems managing their own).

```c
struct psctx_t * ps_alloc(int size);
void ps_free(struct psctx_t * ctx);
void ps_publish(struct psctx_t * ctx, const char * topic, void * pdat);
void ps_subscribe(struct psctx_t * ctx, const char * topic, void (*cb)(void *, void *), void * sdat, int oneshot);
void ps_unsubscribe(struct psctx_t * ctx, const char * topic, void (*cb)(void *, void *), void * sdat);
```

For the instance interface, `ps_subscribe` uses `(cb, sdat)` as the unique identifier, so the same callback may be subscribed to the same topic multiple times with different `sdat` values. `ps_unsubscribe` matches by `(cb, sdat)` exactly to remove the corresponding subscription. The global `psub_*` interface follows the same semantics.

## API Details

### Publish

```c
void psub_publish(const char * topic, void * pdat);
```

Publish a message to `topic`, `pdat` is a pointer to data passed by the publisher. All callbacks subscribed to this topic are invoked in subscription order. If the topic has no subscribers, no operation is performed.

### Subscribe

```c
void psub_subscribe(const char * topic, void (*cb)(void *, void *), void * sdat, int oneshot);
```

Subscribe to `topic`, `cb` is the callback function, `sdat` is the subscriber's private data. A subscription is uniquely identified by the `(cb, sdat)` pair, so the same `cb` can be subscribed multiple times to the same topic with different `sdat` values; duplicate subscriptions of the same `(cb, sdat)` pair are ignored.

- `oneshot = 0` — Persistent subscription, triggered on every publish
- `oneshot = 1` — One-time subscription, automatically cancelled after being triggered once

Callback signature: `void cb(void * pdat, void * sdat)`
- `pdat` — Data passed by the publisher
- `sdat` — Private data passed at subscription time

### Unsubscribe

```c
void psub_unsubscribe(const char * topic, void (*cb)(void *, void *), void * sdat);
```

Cancel subscriptions on `topic`. Two matching modes are supported:

- `cb != NULL` — Exact match by `(cb, sdat)`; only the specified subscription is removed. The `sdat` argument must match the value used at subscription time, otherwise the entry is not removed.
- `cb == NULL` — Wildcard match; removes **all** subscriptions of the given `topic` (the `sdat` argument is ignored in this case).

The topic itself is not deleted; it remains in the context for future subscriptions and is only reclaimed in bulk at `ps_free`.

## Automatic Cleanup

- One-time subscriptions are automatically removed after being triggered
- Context hash table automatically expands on demand
- Topics created on subscription are retained until `ps_free` (they are not individually reclaimed when their subscriber list becomes empty)

## Usage Examples

### Basic publish-subscribe

```c
static void on_network_status(void * pdat, void * sdat)
{
    int * status = (int *)pdat;
    printf("network status: %d\n", *status);
}

psub_subscribe("net/status", on_network_status, NULL, 0);

int status = 1;
psub_publish("net/status", &status);
/* on_network_status is called */
```

### With private data

```c
struct my_app_t {
    int id;
};

static void on_data(void * pdat, void * sdat)
{
    struct my_app_t * app = (struct my_app_t *)sdat;
    char * data = (char *)pdat;
    printf("app%d received: %s\n", app->id, data);
}

struct my_app_t app = { .id = 42 };
psub_subscribe("data/update", on_data, &app, 0);
```

### One-time subscription

```c
static void on_init_done(void * pdat, void * sdat)
{
    printf("initialization complete\n");
}

psub_subscribe("system/init", on_init_done, NULL, 1);
psub_publish("system/init", NULL);   /* Triggered and automatically cancelled */
psub_publish("system/init", NULL);   /* No longer triggered */
```

### Independent context

```c
struct psctx_t * my_ps = ps_alloc(16);

ps_subscribe(my_ps, "event/a", my_callback, NULL, 0);
ps_publish(my_ps, "event/a", data);

ps_free(my_ps);
```

### Bulk unsubscribe

```c
psub_subscribe("net/status", cb1, &ctx1, 0);
psub_subscribe("net/status", cb2, &ctx2, 0);
psub_subscribe("net/status", cb3, NULL, 0);

/* Remove a single subscription by exact match */
psub_unsubscribe("net/status", cb1, &ctx1);

/* Wildcard: remove all remaining subscriptions on this topic */
psub_unsubscribe("net/status", NULL, NULL);
```
