# 蜂鸣器 (buzzer)

蜂鸣器控制接口，支持频率控制和异步蜂鸣模式。

## 设备类型

`DEVICE_TYPE_BUZZER`

## 结构体

```c
struct buzzer_t {
    char * name;
    void (*set)(struct buzzer_t * buzzer, int frequency);
    int (*get)(struct buzzer_t * buzzer);
    void * priv;
};
```

## 核心 API

| 函数 | 说明 |
|------|------|
| `search_buzzer(name)` | 按名称查找蜂鸣器 |
| `search_first_buzzer()` | 查找第一个蜂鸣器 |
| `register_buzzer(buzzer, drv)` | 注册蜂鸣器 |
| `unregister_buzzer(buzzer)` | 注销蜂鸣器 |
| `buzzer_set_frequency(buzzer, frequency)` | 设置蜂鸣频率（0 关闭） |
| `buzzer_get_frequency(buzzer)` | 获取当前频率 |

## 异步 API

基于 `thworker_t` 线程工作队列实现的异步蜂鸣控制。

```c
struct buzzer_async_ctx_t {
    struct buzzer_t * buzzer;
    struct thworker_t * worker;
};
```

| 函数 | 说明 |
|------|------|
| `buzzer_async_ctx_alloc(name)` | 分配异步上下文，name 为 NULL 时使用第一个蜂鸣器 |
| `buzzer_async_ctx_free(ctx)` | 释放异步上下文 |
| `buzzer_async_ctx_clear(ctx)` | 清空工作队列并关闭蜂鸣 |
| `buzzer_async_beep(ctx, frequency, millisecond)` | 异步蜂鸣指定频率和毫秒数 |
| `buzzer_async_play(ctx, rtttl)` | 按 RTTTL 格式异步播放旋律 |

### RTTTL 格式

RTTTL (RingTone Text Transfer Language) 是 Nokia 手机铃声格式，示例：

```
Simpsons:d=4,o=5,b=160:32p,c.6,e6,f#6,8a6,g.6,e6,c6,8a,8f#,8f#,8f#,2g
```

格式为 `<name>:<control>:<tones>`，控制字段：`d`=默认时长，`o`=默认八度，`b`=BPM。

## 驱动

### GPIO 蜂鸣器 (buzzer-gpio)

通过 GPIO 高低电平控制蜂鸣器。

Required properties:
- `gpio`: 蜂鸣器连接的 GPIO 引脚

Optional properties:
- `gpio-config`: GPIO 引脚配置
- `active-low`: 低电平激活蜂鸣器

Example:
```json
"buzzer-gpio": {
    "gpio": 0,
    "gpio-config": -1,
    "active-low": true
}
```

### PWM 蜂鸣器 (buzzer-pwm)

通过 PWM 脉冲宽度调制控制蜂鸣器频率。

Required properties:
- `pwm-name`: 蜂鸣器连接的 PWM 设备名称

Optional properties:
- `pwm-polarity`: PWM 极性，默认 false

Example:
```json
"buzzer-pwm": {
    "pwm-name": "pwm.0",
    "pwm-polarity": false
}
```
