# Random Number Generator (rng)

Hardware random number generator.

## Device Type

`DEVICE_TYPE_RNG`

## Structure

```c
struct rng_t {
    char * name;
    int (*read)(struct rng_t * rng, uint32_t * buf, int len, int timeout);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_rng(name)` | Find RNG by name |
| `register_rng(rng, drv)` | Register an RNG |
| `unregister_rng(rng)` | Unregister an RNG |
| `rng_read(rng, buf, len, timeout)` | Read random numbers |

## Description

Hardware random number generator interface. Provides true random numbers from a hardware entropy source, used for cryptographic key generation and PRNG seeding.
