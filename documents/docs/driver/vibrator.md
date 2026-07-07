# 振动马达 (vibrator)

振动马达控制接口，支持开关控制和异步振动模式。

## 设备类型

`DEVICE_TYPE_VIBRATOR`

## 结构体

```c
struct vibrator_t {
    char * name;
    void (*set)(struct vibrator_t * vib, int state);
    int (*get)(struct vibrator_t * vib);
    void * priv;
};
```

## 核心 API

| 函数 | 说明 |
|------|------|
| `search_vibrator(name)` | 按名称查找振动器 |
| `search_first_vibrator()` | 查找第一个振动器 |
| `register_vibrator(vib, drv)` | 注册振动器 |
| `unregister_vibrator(vib)` | 注销振动器 |
| `vibrator_set_state(vib, state)` | 设置振动状态（0 关闭，1 开启） |
| `vibrator_get_state(vib)` | 获取振动状态 |

## 异步 API

基于 `thworker_t` 线程工作队列实现的异步振动控制。

```c
struct vibrator_async_ctx_t {
    struct vibrator_t * vibrator;
    struct thworker_t * worker;
};
```

| 函数 | 说明 |
|------|------|
| `vibrator_async_ctx_alloc(name)` | 分配异步上下文，name 为 NULL 时使用第一个振动器 |
| `vibrator_async_ctx_free(ctx)` | 释放异步上下文 |
| `vibrator_async_ctx_clear(ctx)` | 清空工作队列并关闭振动 |
| `vibrator_async_vibrate(ctx, state, millisecond)` | 异步振动指定毫秒数 |
| `vibrator_async_play(ctx, morse)` | 按摩尔斯电码模式异步振动 |

### 摩尔斯电码时序

| 符号 | 振动时长 | 停顿时长 |
|------|---------|---------|
| `.` (点) | 100ms | 100ms |
| `-` (划) | 300ms | 100ms |
| 字符间隔 | — | 300ms |
| 单词间隔 | — | 700ms |

## 驱动

### GPIO 振动器 (vibrator-gpio)

通过 GPIO 高低电平控制振动马达。

Required properties:
- `gpio`: 振动器连接的 GPIO 引脚

Optional properties:
- `gpio-config`: GPIO 引脚配置
- `active-low`: 低电平激活振动器

Example:
```json
"vibrator-gpio": {
    "gpio": 0,
    "gpio-config": -1,
    "active-low": true
}
```

### PWM 振动器 (vibrator-pwm)

通过 PWM 脉冲宽度调制控制振动马达。

Required properties:
- `pwm-name`: 振动器连接的 PWM 设备名称

Optional properties:
- `pwm-period-ns`: PWM 周期（纳秒），默认 2272727
- `pwm-polarity`: PWM 极性，默认 true

Example:
```json
"vibrator-pwm": {
    "pwm-name": "pwm.0",
    "pwm-period-ns": 2272727,
    "pwm-polarity": true
}
```
