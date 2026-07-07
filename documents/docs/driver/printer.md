# 打印机 (printer)

打印机。

## 设备类型

`DEVICE_TYPE_PRINTER`

## 结构体

```c
struct printer_t {
    char * name;
    int (*print)(struct printer_t * printer, uint8_t * buf, int64_t len);
    int (*feed)(struct printer_t * printer, int lines);
    int (*cut)(struct printer_t * printer);
    int (*standby)(struct printer_t * printer, int ms);
    int (*ioctl)(struct printer_t * printer, const char * cmd, void * arg);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_printer(name)` | 按名称查找打印机 |
| `register_printer(printer, drv)` | 注册打印机 |
| `unregister_printer(printer)` | 注销打印机 |
| `printer_print(raw, len)` | 打印原始数据 |
| `printer_feed(lines)` | 进纸 |
| `printer_cut()` | 切纸 |
| `printer_print_text/surface/qrcode()` | 高级打印功能 |

## 说明

热敏/票据打印机接口。支持原始数据打印、进纸、切纸。包含高级打印助手：格式化文本、二维码、图像打印和 surface 渲染。异步变体支持非阻塞打印。
