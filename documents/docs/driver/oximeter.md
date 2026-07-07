# 血氧仪 (oximeter)

脉搏血氧仪。

## 设备类型

`DEVICE_TYPE_OXIMETER`

## 结构体

```c
struct oximeter_t {
    char * name;
    int (*get)(struct oximeter_t * oximeter, int * sp02, int * rate);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_oximeter(name)` | 按名称查找血氧仪 |
| `register_oximeter(oximeter, drv)` | 注册血氧仪 |
| `unregister_oximeter(oximeter)` | 注销血氧仪 |
| `oximeter_get(oximeter, sp02, rate)` | 获取血氧饱和度和心率 |

## 说明

脉搏血氧仪传感器接口（如 MAX30100/MAX30102）。通过 PPG 传感器返回血氧饱和度（SpO2）和心率。用于可穿戴健康监测。
