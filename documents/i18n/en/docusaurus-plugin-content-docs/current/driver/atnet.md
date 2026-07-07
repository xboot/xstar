# AT Network Interface (atnet)

AT command network interface.

## Device Type

`DEVICE_TYPE_ATNET`

## Structure

```c
struct atnet_t {
    char * name;
    struct atnet_tcp_t * (*tcp_create)(struct atnet_t * atnet, int type);
    struct atnet_udp_t * (*udp_create)(struct atnet_t * atnet);
    int (*ioctl)(struct atnet_t * atnet, const char * cmd, void * arg);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_atnet(name)` | Find an ATNET device by name |
| `search_first_atnet()` | Find the first ATNET device |
| `register_atnet(atnet, drv)` | Register an ATNET device |
| `unregister_atnet(atnet)` | Unregister an ATNET device |

## Description

Network interface driver based on AT commands (GSM/ESP8266 modules). Provides TCP, UDP and SSL socket abstraction through serial AT commands.
