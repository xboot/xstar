# 网络接口 (net)

网络接口。

## 设备类型

`DEVICE_TYPE_NET`

## 结构体

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

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_net(name)` | 按名称查找网络设备 |
| `register_net(net, drv)` | 注册网络设备 |
| `unregister_net(net)` | 注销网络设备 |
| `net_listen/accept/connect(net, ...)` | 网络连接管理 |
| `net_read/write(net, c, buf, len)` | 网络数据读写 |

## 说明

网络栈接口，在任何传输层（以太网、WiFi、PPP 等）上提供类似 TCP 的套接字操作。支持服务端和客户端模式。
