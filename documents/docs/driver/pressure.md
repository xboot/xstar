# 气压传感器 (pressure)

气压传感器。

## 设备类型

`DEVICE_TYPE_PRESSURE`

## 结构体

```c
struct pressure_t {
    char * name;
    int (*get)(struct pressure_t * pressure, int * pa, int * temp);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_pressure(name)` | 按名称查找气压传感器 |
| `register_pressure(pressure, drv)` | 注册气压传感器 |
| `unregister_pressure(pressure)` | 注销气压传感器 |
| `pressure_get(pressure, pa, temp)` | 获取气压和温度 |

## 说明

气压和温度传感器接口（如 BMP180/BMP280）。用于海拔估计和天气监测。
