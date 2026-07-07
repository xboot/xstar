# 输入设备 (input)

输入设备。

## 设备类型

`DEVICE_TYPE_INPUT`

## 结构体

```c
struct input_t {
    char * name;
    int (*ioctl)(struct input_t * input, const char * cmd, void * arg);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_input(name)` | 按名称查找输入设备 |
| `register_input(input, drv)` | 注册输入设备 |
| `unregister_input(input)` | 注销输入设备 |
| `input_ioctl(input, cmd, arg)` | 控制输入设备 |

## 说明

通用输入设备接口（如触摸屏、按键、键盘）。使用字符串 ioctl 控制模型。实际输入事件生成由内核输入子系统处理。
