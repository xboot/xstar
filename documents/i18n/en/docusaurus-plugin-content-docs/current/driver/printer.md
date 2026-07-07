# Printer (printer)

Printer.

## Device Type

`DEVICE_TYPE_PRINTER`

## Structure

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

## Key API

| Function | Description |
|------|------|
| `search_printer(name)` | Find printer by name |
| `register_printer(printer, drv)` | Register printer |
| `unregister_printer(printer)` | Unregister printer |
| `printer_print(raw, len)` | Print raw data |
| `printer_feed(lines)` | Feed paper |
| `printer_cut()` | Cut paper |
| `printer_print_text/surface/qrcode()` | Advanced printing functions |

## Description

Thermal/receipt printer interface. Supports raw data printing, paper feeding, and paper cutting. Includes advanced print helpers: formatted text, QR codes, image printing, and surface rendering. Async variants support non-blocking printing.
