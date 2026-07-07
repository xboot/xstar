# 环境光传感器 (light)

环境光传感器。

## 设备类型

`DEVICE_TYPE_LIGHT`

## 结构体

```c
struct light_t {
    char * name;
    int (*get)(struct light_t * light);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_light(name)` | 按名称查找光传感器 |
| `register_light(light, drv)` | 注册光传感器 |
| `unregister_light(light)` | 注销光传感器 |
| `light_get_illuminance(light)` | 获取光照度 |

## 说明

环境光传感器接口。返回当前光照度（lux），用于自动亮度调节。
