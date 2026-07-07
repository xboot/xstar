# 加速度计 (gmeter)

加速度计。

## 设备类型

`DEVICE_TYPE_GMETER`

## 结构体

```c
struct gmeter_t {
    char * name;
    int (*get)(struct gmeter_t * gmeter, int * x, int * y, int * z);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_gmeter(name)` | 按名称查找加速度计 |
| `register_gmeter(gmeter, drv)` | 注册加速度计 |
| `unregister_gmeter(gmeter)` | 注销加速度计 |
| `gmeter_get_acceleration(gmeter, x, y, z)` | 获取三轴加速度值 |

## 说明

加速度计传感器接口。返回三轴加速度值，用于运动检测和方向感知。
