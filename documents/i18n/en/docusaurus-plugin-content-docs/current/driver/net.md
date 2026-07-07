# Network Interface (net)

Network interface.

## Device Type

`DEVICE_TYPE_NET`

## Structure

```c
struct net_t {
    char * name;
    struct socket_listen_t * (*listen)(struct net_t * net, int type, int port);
    struct socket_connect_t * (*accept)(struct net_t * net, struct socket_listen_t * l, int type);
    struct socket_connect_t * (*connect)(struct net_t * net, int type, const char * host, int port);
    int (*read)(struct net_t * net, struct socket_connect_t * c, uint8_t * buf, int64_t len);
    int (*write)(struct net_t * net, struct socket_connect_t * c, uint8_t * buf, int64_t len);
    int (*close)(struct net_t * net, struct socket_connect_t * c);
    int (*delete)(struct net_t * net, struct socket_listen_t * l);
    int (*ioctl)(struct net_t * net, const char * cmd, void * arg);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_net(name)` | Find network device by name |
| `register_net(net, drv)` | Register network device |
| `unregister_net(net)` | Unregister network device |
| `net_listen/accept/connect(net, ...)` | Network connection management |
| `net_read/write(net, c, buf, len)` | Network data read/write |

## Description

Network stack interface, providing TCP-like socket operations over any transport layer (Ethernet, WiFi, PPP, etc.). Supports both server and client modes.
