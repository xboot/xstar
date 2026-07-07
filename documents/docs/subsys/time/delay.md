# 延时 (delay)

基于时钟源（clocksource）的忙等待延时函数，提供纳秒、微秒、毫秒三种精度的延时，适用于短时间精确等待场景。

## 工作原理

延时函数通过 `ktime_get()` 获取当前时间，计算目标超时时间，然后忙等待轮询直到超时：

```c
void udelay(uint32_t us)
{
    ktime_t timeout = ktime_add_us(ktime_get(), us);
    while(ktime_before(ktime_get(), timeout));
}
```

三种延时函数的实现完全一致，仅时间单位不同：

- `ndelay()` 使用 `ktime_add_ns()` 添加纳秒
- `udelay()` 使用 `ktime_add_us()` 添加微秒
- `mdelay()` 使用 `ktime_add_ms()` 添加毫秒

## API

| 函数 | 说明 |
|------|------|
| `ndelay(ns)` | 纳秒级忙等待延时 |
| `udelay(us)` | 微秒级忙等待延时 |
| `mdelay(ms)` | 毫秒级忙等待延时 |

## 用法示例

```c
#include <kernel/time/delay.h>

/* 硬件复位后等待 10 微秒 */
udelay(10);

/* I2C 时序延时 */
ndelay(500);

/* 短时间等待 5 毫秒 */
mdelay(5);
```

## 说明

- 延时为忙等待（busy-wait），期间 CPU 不会释放，不适用于长时间等待
- 延时精度取决于时钟源设备的分辨率，通过 `ktime_get()` 获取纳秒级时间戳
- 长时间等待应使用定时器（`timer`）或线程休眠（`xos_thread_msleep`）
- 在中断上下文中可以安全使用，因为不涉及调度
