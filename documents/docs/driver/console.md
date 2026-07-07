# 控制台 (console)

控制台串口。

## 设备类型

`DEVICE_TYPE_CONSOLE`

## 结构体

```c
struct console_t {
    char * name;
    ssize_t (*read)(struct console_t * console, uint8_t * buf, size_t count);
    ssize_t (*write)(struct console_t * console, uint8_t * buf, size_t count);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_console(name)` | 按名称查找控制台 |
| `register_console(console, drv)` | 注册控制台 |
| `unregister_console(console)` | 注销控制台 |
| `console_read(console, buf, count)` | 读取数据 |
| `console_write(console, buf, count)` | 写入数据 |

## 说明

字符型控制台 I/O 设备（串行终端），用于交互式 Shell 和日志输出。
