# 磁力计 (compass)

磁力计/指南针。

## 设备类型

`DEVICE_TYPE_COMPASS`

## 结构体

```c
struct compass_t {
    char * name;
    int ox, oy, oz;
    int (*get)(struct compass_t * compass, int * x, int * y, int * z);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_compass(name)` | 按名称查找指南针 |
| `register_compass(compass, drv)` | 注册指南针 |
| `unregister_compass(compass)` | 注销指南针 |
| `compass_get_magnetic(compass, x, y, z)` | 获取磁场值 |
| `compass_get_heading(compass, declination)` | 获取航向角 |

## 说明

磁力计传感器接口。返回三轴磁场读数，支持硬铁校准偏移配置。
