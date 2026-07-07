# 陀螺仪 (gyroscope)

陀螺仪。

## 设备类型

`DEVICE_TYPE_GYROSCOPE`

## 结构体

```c
struct gyroscope_t {
    char * name;
    int (*get)(struct gyroscope_t * gyroscope, int * x, int * y, int * z);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_gyroscope(name)` | 按名称查找陀螺仪 |
| `register_gyroscope(gyro, drv)` | 注册陀螺仪 |
| `unregister_gyroscope(gyro)` | 注销陀螺仪 |
| `gyroscope_get_palstance(gyro, x, y, z)` | 获取三轴角速度 |

## 说明

陀螺仪传感器接口。返回三轴角速度，用于方向跟踪。通常与加速度计配合用于 IMU。
