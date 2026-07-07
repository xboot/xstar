# 直流电机 (motor)

直流电机。

## 设备类型

`DEVICE_TYPE_MOTOR`

## 结构体

```c
struct motor_t {
    char * name;
    int (*enable)(struct motor_t * motor);
    int (*disable)(struct motor_t * motor);
    int (*set)(struct motor_t * motor, int speed);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_motor(name)` | 按名称查找电机 |
| `register_motor(motor, drv)` | 注册电机 |
| `unregister_motor(motor)` | 注销电机 |
| `motor_enable/disable(motor)` | 使能/禁用电机 |
| `motor_set_speed(motor, speed)` | 设置转速 |
| `motor_async_play(ctx, ...)` | 异步定时运行 |

## 说明

直流电机控制接口。使能/禁用电机并设置旋转速度。异步上下文支持非阻塞定时操作。
