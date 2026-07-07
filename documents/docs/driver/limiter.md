# 限位开关 (limiter)

限位开关 / 终点传感器 (endstop) 子系统。返回二进制状态，指示机械限位是否到达，常用于 CNC、3D 打印机、机器人、自动门、料位检测等场景。

## 驱动框架

### 设备类型

`DEVICE_TYPE_LIMITER`

### 核心数据结构

```c
struct limiter_t {
    char * name;
    int (*get)(struct limiter_t * limiter);
    void * priv;
};
```

- `name`：设备名称，全局唯一，可通过 `search_limiter()` 检索。
- `get`：由具体驱动实现的状态读取回调，返回 `1` 表示限位触发，`0` 表示未触发。
- `priv`：具体驱动的私有数据指针。

### 核心 API

| 函数 | 说明 |
|------|------|
| `search_limiter(name)` | 按名称查找限位器 |
| `register_limiter(limiter, drv)` | 注册限位器，自动创建 sysfs 节点 |
| `unregister_limiter(limiter)` | 注销限位器 |
| `limiter_get_status(limiter)` | 获取当前限位状态 (主动查询) |

### sysfs 节点

注册时核心层自动创建 `status` 只读节点，可在调试 shell 中通过 kobj 虚拟文件系统查看实时状态。

### 事件通知机制

limiter 子系统约定使用 **psub (publish/subscribe，发布/订阅)** 模式将状态变化以异步事件的方式通知上层应用，避免上层主动轮询。

#### 事件主题 (Topic)

| 事件主题 | 触发时机 | 数据 |
|----------|----------|------|
| `limiter.triggered` | 限位被触发 (status 由 0 变为 1) | `struct limiter_t *` |
| `limiter.released`  | 限位被释放 (status 由 1 变为 0) | `struct limiter_t *` |

事件携带的数据指针为 `struct limiter_t *`，订阅者可通过 `limiter->name` 区分多路限位开关。

#### 订阅端示例 (Application)

上层应用通过 `psub_subscribe()` 注册回调即可接收事件：

```c
#include <kernel/core/psub.h>
#include <driver/limiter/limiter.h>

static void on_limiter_triggered(const char * topic, void * data, void * priv)
{
    struct limiter_t * limiter = (struct limiter_t *)data;
    /* 紧急停机、轴归零完成、料满检测等业务逻辑 */
}

static void on_limiter_released(const char * topic, void * data, void * priv)
{
    struct limiter_t * limiter = (struct limiter_t *)data;
    /* 恢复运行 */
}

psub_subscribe("limiter.triggered", on_limiter_triggered, NULL);
psub_subscribe("limiter.released",  on_limiter_released,  NULL);
```

#### 设计要点

- **解耦**：驱动只负责发布事件，无需感知订阅者数量，便于运动控制、HMI、日志等多模块同时响应。
- **沿触发**：仅在状态翻转的边沿发布事件，不会重复发送冗余事件；上层若需要瞬时状态可调用 `limiter_get_status()` 主动查询。
- **可移植**：不同的具体驱动 (轮询式、中断式、I2C IO 扩展等) 只要遵循相同的事件主题约定，上层代码无需改动。

## 具体驱动：limiter-gpio-polled

文件：`xstar/driver/limiter/limiter-gpio-polled.c`
配置：`CONFIG_DRV_LIMITER_GPIO_POLLED`

基于 GPIO 软件定时器轮询实现的限位开关驱动，适用于没有外部中断资源、或希望统一在软件层做去抖处理的场景。

### 设备树配置

在 `/romdisk/boot/boot.json` 中以 `limiter-gpio-polled` 为节点名添加配置：

```json
"limiter-gpio-polled@0": {
    "gpio":              123,
    "gpio-config":       -1,
    "active-low":        true,
    "poll-interval-ms":  10
}
```

| 字段 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `gpio` | int | -1 (必填) | 输入 GPIO 编号，需 `gpio_is_valid()` 通过 |
| `gpio-config` | int | -1 (不修改) | 平台相关的复用功能配置 (pinmux) |
| `active-low` | bool | false | 低电平触发；为 `true` 时驱动自动启用上拉，为 `false` 时启用下拉 |
| `poll-interval-ms` | int | 10 | 轮询周期 (毫秒)，决定响应延迟与去抖窗口 |

驱动在 probe 阶段会：

1. 校验 GPIO 合法性；
2. 将 GPIO 配置为输入并按 `active-low` 设置上/下拉；
3. 读取初始电平作为初始状态，并填充去抖缓冲区；
4. 启动周期定时器开始轮询。

### 事件触发流程 (Event-driven)

驱动通过定时器周期采样 + 软件去抖 + 边沿检测的方式发布事件：

```c
static int limiter_gpio_polled_timer_function(struct timer_t * timer, void * data)
{
    struct limiter_t * limiter = (struct limiter_t *)(data);
    struct limiter_gpio_polled_pdata_t * pdat = limiter->priv;
    int status;

    if(gpio_get_value(pdat->gpio))
        status = pdat->active_low ? 0 : 1;
    else
        status = pdat->active_low ? 1 : 0;

    pdat->buffer[pdat->bufidx] = status;
    pdat->bufidx = (pdat->bufidx + 1) & 0x3;

    if(pdat->status != status)
    {
        if((pdat->buffer[0] == status) && (pdat->buffer[1] == status) &&
           (pdat->buffer[2] == status) && (pdat->buffer[3] == status))
        {
            pdat->status = status;
            if(status)
                psub_publish("limiter.triggered", limiter);
            else
                psub_publish("limiter.released", limiter);
        }
    }
    timer_forward(timer, ms_to_ktime(pdat->interval));
    return 1;
}
```

工作流程：

1. **轮询采样**：定时器以 `poll-interval-ms` 周期读取 GPIO 电平，根据 `active-low` 计算逻辑状态。
2. **软件去抖**：4 个采样点的环形缓冲区 (`buffer[4]`)，仅当连续 4 次采样结果一致且与上一稳定状态不同时，才认定为有效状态变化，有效滤除机械抖动与电气毛刺。
3. **边沿事件发布**：状态确认变化后，调用 `psub_publish()` 发布 `limiter.triggered` 或 `limiter.released`。

> 响应延迟约为 `poll-interval-ms × 4`。可根据应用场景在响应速度 (减小周期) 与抗抖能力 (增大周期) 之间权衡。

### 主动查询 (Polling Query)

除事件方式外，上层随时可主动获取当前稳定状态：

```c
struct limiter_t * lim = search_limiter("limiter-gpio-polled.0");
if(lim && limiter_get_status(lim))
{
    /* 当前处于触发状态 */
}
```

`limiter_get_status()` 最终调用驱动的 `get()` 回调，返回的是经过去抖确认后的稳定状态 `pdat->status`，不会受瞬时抖动影响。

### 电源管理

- `suspend`：取消轮询定时器，停止采样。
- `resume`：重新启动定时器，按当前 GPIO 电平继续采样 (注意：跨 suspend 期间的电平变化不会补发事件，唤醒后以新的稳定状态为准)。
