# 串口 (uart)

通用异步收发器。

## 设备类型

`DEVICE_TYPE_UART`

## 结构体

```c
struct uart_t {
    char * name;
    int (*set)(struct uart_t * uart, int baud, int data, int parity, int stop);
    int (*get)(struct uart_t * uart, int * baud, int * data, int * parity, int * stop);
    ssize_t (*read)(struct uart_t * uart, uint8_t * buf, size_t count);
    ssize_t (*write)(struct uart_t * uart, uint8_t * buf, size_t count);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_uart(name)` | 按名称查找 UART |
| `register_uart(uart, drv)` | 注册 UART |
| `unregister_uart(uart)` | 注销 UART |
| `uart_set(uart, baud, data, parity, stop)` | 配置串口参数 |
| `uart_read/write(uart, buf, count)` | 读写数据 |

## 说明

通用异步收发器接口。配置波特率、数据位、校验位和停止位，提供阻塞读写操作用于字节流通信。
