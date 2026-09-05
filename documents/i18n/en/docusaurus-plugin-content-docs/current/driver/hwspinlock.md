# Hardware Mutex Lock (hwspinlock)

Hardware mutex lock.

## Device Type

`DEVICE_TYPE_HWSPINLOCK`

## Structure

```c
struct hwspinlock_t {
    char * name;
    int base, nlock;
    int (*trylock)(struct hwspinlock_t * hsl, int lock);
    void (*lock)(struct hwspinlock_t * hsl, int lock);
    void (*unlock)(struct hwspinlock_t * hsl, int lock);
    void * priv;
};

struct hwspinlock_desc_t {
    struct hwspinlock_t * hsl;
    int offset;
};
```

## Key API

| Function | Description |
|------|------|
| `search_hwspinlock(lock)` | Find a hardware mutex lock device by global lock number |
| `register_hwspinlock(hsl, drv)` | Register a hardware mutex lock device |
| `unregister_hwspinlock(hsl)` | Unregister a hardware mutex lock device |
| `hwspinlock_is_valid(lock)` | Check whether a global lock number is valid |
| `hwspinlock_trylock(lock)` | Try to acquire the specified lock |
| `hwspinlock_lock(lock)` | Acquire the specified lock (busy-wait) |
| `hwspinlock_unlock(lock)` | Release the specified lock |
| `hwspinlock_desc_alloc(lock)` | Allocate a lock descriptor |
| `hwspinlock_desc_free(desc)` | Free a lock descriptor |
| `hwspinlock_desc_trylock/lock/unlock(desc)` | Operate on the lock via descriptor |

## Description

Hardware mutex lock interface. Each device manages a globally numbered lock space (`base` ~ `base + nlock - 1`), described in the device tree via the `hwspinlock-base` and `hwspinlock-count` properties. Intended for mutual exclusion between processors, and between processors and hardware modules in AMP systems — distinct from `spinlock`, which targets SMP in-kernel mutual exclusion. The `hwspinlock_desc_t` descriptor caches the device pointer and lock offset, suitable for high-frequency locking scenarios by avoiding a global lock number lookup on every operation.
