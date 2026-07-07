# 脉宽调制器 (pwm)

脉宽调制器。

## 设备类型

`DEVICE_TYPE_PWM`

## 结构体

```c
struct pwm_t {
    char * name;
    int (*config)(struct pwm_t * pwm, int duty, int period, int polarity);
    int (*enable)(struct pwm_t * pwm);
    int (*disable)(struct pwm_t * pwm);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_pwm(name)` | 按名称查找 PWM |
| `register_pwm(pwm, drv)` | 注册 PWM |
| `unregister_pwm(pwm)` | 注销 PWM |
| `pwm_config(pwm, duty, period, polarity)` | 配置 PWM |
| `pwm_enable/disable(pwm)` | 使能/禁用 PWM |

## 说明

脉宽调制器接口。配置 PWM 输出的占空比、周期和极性。用于电机控制、LED 调光和音调生成。
