# 接近传感器 (proximity)

接近传感器。

## 设备类型

`DEVICE_TYPE_PROXIMITY`

## 结构体

```c
struct proximity_t {
    char * name;
    int (*get)(struct proximity_t * proximity);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_proximity(name)` | 按名称查找接近传感器 |
| `register_proximity(prox, drv)` | 注册接近传感器 |
| `unregister_proximity(prox)` | 注销接近传感器 |
| `proximity_get_distance(prox)` | 获取距离 |

## 说明

接近传感器接口。从红外或超声波传感器返回距离读数，用于物体检测和手势感知。
