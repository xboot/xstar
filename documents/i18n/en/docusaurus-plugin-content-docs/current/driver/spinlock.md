# Hardware Spinlock (spinlock)

Hardware spinlock.

## Device Type

`DEVICE_TYPE_SPINLOCK`

## Structure

```c
struct spinlock_t { int v; };
```

## Key API

| Function | Description |
|------|------|
| `search_spinlock(name)` | Find spinlock device by name |
| `register_spinlock(sl, drv)` | Register a spinlock device |
| `unregister_spinlock(sl)` | Unregister a spinlock device |
| `spinlock_init(lock)` | Initialize a spinlock |
| `spinlock_lock(lock)` | Acquire a spinlock |
| `spinlock_trylock(lock)` | Try to acquire a spinlock |
| `spinlock_unlock(lock)` | Release a spinlock |

## Description

Hardware spinlock interface for inter-processor synchronization in SMP systems. Distinct from atomic operation devices — spinlocks are dedicated for mutual exclusion in multi-core scenarios.
