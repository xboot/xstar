# 舵机 (servo)

舵机。

## 设备类型

`DEVICE_TYPE_SERVO`

## 结构体

```c
struct servo_t {
    char * name;
    int (*enable)(struct servo_t * servo);
    int (*disable)(struct servo_t * servo);
    int (*range)(struct servo_t * servo);
    int (*get)(struct servo_t * servo);
    int (*set)(struct servo_t * servo, int angle);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_servo(name)` | 按名称查找舵机 |
| `register_servo(servo, drv)` | 注册舵机 |
| `unregister_servo(servo)` | 注销舵机 |
| `servo_enable/disable(servo)` | 使能/禁用舵机 |
| `servo_set/get_angle(servo, angle)` | 设置/获取角度 |

## 说明

舵机控制接口。控制舵机在角度范围内的位置，用于机器人、相机云台和自动化。
