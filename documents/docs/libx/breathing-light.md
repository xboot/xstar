# 呼吸灯 (breathing)

呼吸灯亮度计算模块，根据当前时间返回对应的亮度值，模拟自然呼吸的渐亮渐灭效果。

## 原理

基于预计算的 1024 点 8-bit 查找表，曲线符合人眼感知的对数特性，低亮度区变化缓慢、高亮度区变化迅速，视觉上呈现平滑的呼吸节奏。查表值内部存储为 `[0, 255]`，返回时通过整数缩放映射到 `[0, 1000]`。

输入时间自动取模循环，无需手动处理周期边界。

## API

```c
int breathing(int ms);
```

传入当前时间（毫秒），返回亮度值，范围 `[0, 1000]`。

- 0 表示最暗（灭）
- 1000 表示最亮
- 周期为 4096ms（约 4 秒），即 `ms` 每 4096 循环一次

## 曲线特性

- 上升沿约 1388ms，下降沿约 2672ms，升降比约 1:1.93
- 模拟自然呼吸节奏：吸气快、呼气慢
- 顶部平滑过渡，无突变
- 低亮度区间占比更大，视觉上"慢起慢落"

### 数学模型

查找表数据最佳拟合为非对称 sin^p 曲线，以索引 i（0 ≤ i < 1024）为自变量：

```
y(i) = 1000 × sin^pr(π × i / (2 × Tr))    i ≤ Tr（上升段）
y(i) = 1000 × sin^pf(π × (1024 - i) / (2 × Tf))    i > Tr（下降段）
```

| 参数 | 值 | 对应时间 |
|------|------|----------|
| Tr（上升半周期） | 349 | 1396ms |
| Tf（下降半周期） | 675 | 2700ms |
| pr（上升指数） | 4.6 | - |
| pf（下降指数） | 4.8 | - |

拟合均方根误差（RMSE）≈ 12，最大绝对误差 ≈ 23。

## 多灯同步

`breathing()` 是一个 **无状态纯函数**，仅根据传入的时间参数查表返回亮度值，内部无任何全局状态、计数器或时间记忆。这一特性使得多个呼吸灯之间能够天然保持完全同步的呼吸节奏，无需任何锁、信号量或额外协调机制。

### 同步原理

- 函数输出仅由输入时间决定：相同的 `ms` 输入永远得到相同的亮度输出
- 所有呼吸灯只需引用 **同一时间基准**（如系统单调时钟 `ktime_get()`），调用 `breathing(ms)` 时传入相同的 `ms`，则所有灯的亮度变化在任意时刻都完全一致
- 由于无累积误差，无论运行多长时间，多灯之间不会出现相位漂移

### 相位偏移

通过为每个灯添加一个固定的时间偏移量，可以实现"流水"或"波浪"效果，且各灯之间的相位差始终保持稳定：

- `breathing(ms + 1024)`：相位偏移 1/4 周期
- `breathing(ms + 2048)`：相位偏移 1/2 周期
- `breathing(ms + 3072)`：相位偏移 3/4 周期

### 多灯同步示例

```c
void multi_led_breathing_loop(void)
{
    ktime_t start = ktime_get();
    while(1)
    {
        int ms = (int)(ktime_to_ms(ktime_get() - start));

        led_set_pwm(0, breathing(ms));
        led_set_pwm(1, breathing(ms));
        led_set_pwm(2, breathing(ms + 1024));
        led_set_pwm(3, breathing(ms + 2048));

        xos_msleep(16);
    }
}
```

上述示例中，LED0 与 LED1 完全同步，LED2、LED3 与 LED0 之间分别保持 1/4、1/2 周期的固定相位差，形成稳定的视觉节奏。

## 使用示例

### LED 呼吸灯

```c
void led_breathing_loop(void)
{
    ktime_t start = ktime_get();
    while(1)
    {
        int ms = (int)(ktime_to_ms(ktime_get() - start));
        int brightness = breathing(ms);
        led_set_pwm(brightness);
        xos_msleep(16);
    }
}
```

### 与 PWM 配合

```c
int duty = breathing(ktime_to_ms(ktime_get()));
int pwm_period = 1000;
pwm_config(pwm_period, duty);
```

亮度值 0~1000 可直接映射为 PWM 占空比（周期设为 1000），实现无额外换算的亮度控制。
