# 温度传感器 (thermometer)

温度传感器。

## 设备类型

`DEVICE_TYPE_THERMOMETER`

## 结构体

```c
struct thermometer_t {
    char * name;
    int (*get)(struct thermometer_t * thermometer);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_thermometer(name)` | 按名称查找温度传感器 |
| `register_thermometer(t, drv)` | 注册温度传感器 |
| `unregister_thermometer(t)` | 注销温度传感器 |
| `thermometer_get(t)` | 获取温度值 |

## 说明

温度传感器接口。返回温度读数（毫摄氏度），用于系统热监测和环境感测。
