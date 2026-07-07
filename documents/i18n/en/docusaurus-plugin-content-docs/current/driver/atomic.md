# Hardware Atomic Operations (atomic)

Hardware atomic operations.

## Device Type

`DEVICE_TYPE_ATOMIC`

## Structure

```c
struct atomic_t { long v; };
```

## Key API

| Function | Description |
|------|------|
| `search_atomic(name)` | Find an atomic device by name |
| `register_atomic(atomic, drv)` | Register an atomic device |
| `unregister_atomic(atomic)` | Unregister an atomic device |
| `atomic_set(atomic, v)` | Set value |
| `atomic_get(atomic)` | Get value |
| `atomic_add(atomic, v)` | Atomic add |
| `atomic_sub(atomic, v)` | Atomic subtract |
| `atomic_cas(atomic, o, n)` | Compare and swap |

## Description

Hardware-supported atomic operations. May require special hardware support or memory barriers on certain architectures.
