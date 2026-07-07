# AT命令网络接口 (atnet)

AT 命令网络接口。

## 设备类型

`DEVICE_TYPE_ATNET`

## 结构体

```c
struct atnet_t {
    char * name;
    struct atnet_tcp_t * (*tcp_create)(struct atnet_t * atnet, int type);
    struct atnet_udp_t * (*udp_create)(struct atnet_t * atnet);
    int (*ioctl)(struct atnet_t * atnet, const char * cmd, void * arg);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_atnet(name)` | 按名称查找 ATNET 设备 |
| `search_first_atnet()` | 查找第一个 ATNET 设备 |
| `register_atnet(atnet, drv)` | 注册 ATNET 设备 |
| `unregister_atnet(atnet)` | 注销 ATNET 设备 |

## 说明

基于 AT 命令的网络接口驱动（GSM/ESP8266 模块）。通过串口 AT 命令提供 TCP、UDP 和 SSL 套接字抽象。
