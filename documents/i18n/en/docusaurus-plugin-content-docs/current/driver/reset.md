# Reset Controller (reset)

Reset controller.

## Device Type

`DEVICE_TYPE_RESETCHIP`

## Structure

```c
struct resetchip_t {
    char * name;
    int base, nreset;
    int (*assert)(struct resetchip_t * chip, int offset);
    int (*deassert)(struct resetchip_t * chip, int offset);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_resetchip(name)` | Find reset controller by name |
| `register_resetchip(chip, drv)` | Register a reset controller |
| `unregister_resetchip(chip)` | Unregister a reset controller |
| `reset_assert/deassert(chip, offset)` | Assert/deassert reset |
| `reset_reset(chip, offset, ms)` | Pulse reset |

## Description

Hardware reset line controller interface. Supports assertion of individual reset signals and configurable duration pulse reset. The `resets_t` aggregate type manages multiple reset lines in bulk.
