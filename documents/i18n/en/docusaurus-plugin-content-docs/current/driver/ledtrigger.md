# LED Trigger (ledtrigger)

LED trigger.

## Device Type

`DEVICE_TYPE_LEDTRIGGER`

## Structure

```c
struct ledtrigger_t {
    char * name;
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_ledtrigger(name)` | Find LED trigger by name |
| `register_ledtrigger(trig, drv)` | Register LED trigger |
| `unregister_ledtrigger(trig)` | Unregister LED trigger |

## Description

LED trigger associated devices. Links named triggers (e.g., "heartbeat", "mmc") to LEDs, causing LEDs to automatically change state based on system events.
