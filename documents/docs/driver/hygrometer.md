# 温湿度传感器 (hygrometer)

温湿度传感器。

## 设备类型

`DEVICE_TYPE_HYGROMETER`

## 结构体

```c
struct hygrometer_t {
    char * name;
    int (*get)(struct hygrometer_t * hygrometer, int * temp, int * humi);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_hygrometer(name)` | 按名称查找温湿度传感器 |
| `register_hygrometer(hygro, drv)` | 注册温湿度传感器 |
| `unregister_hygrometer(hygro)` | 注销温湿度传感器 |
| `hygrometer_get(hygro, temp, humi)` | 获取温度和湿度 |

## 说明

温湿度传感器接口。返回相对湿度和温度读数，用于环境监测。
