# UART (uart)

Universal asynchronous receiver-transmitter.

## Device Type

`DEVICE_TYPE_UART`

## Structure

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

## Key API

| Function | Description |
|------|------|
| `search_uart(name)` | Find UART by name |
| `register_uart(uart, drv)` | Register a UART |
| `unregister_uart(uart)` | Unregister a UART |
| `uart_set(uart, baud, data, parity, stop)` | Configure serial port parameters |
| `uart_read/write(uart, buf, count)` | Read/write data |

## Description

Universal asynchronous receiver-transmitter interface. Configures baud rate, data bits, parity and stop bits, provides blocking read/write operations for byte stream communication.
