# 退避策略 (backoff)

指数退避加随机抖动的重试延迟计算模块，用于网络请求、设备重连等需要逐步增加重试间隔的场景。

## 原理

每次调用 `backoff_next()` 时，先基于当前 `delay` 计算一个 `[0, delay]` 范围内的随机值作为返回值，然后再更新 `delay`（指数增长）。随机抖动可以避免多个客户端同时重试造成"惊群"。

## 数据结构

```c
struct backoff_t {
    int max_delay;   /* 最大延迟上限（毫秒） */
    int max_retry;   /* 最大重试次数，<=0 表示无限制 */
    int delay;       /* 当前延迟基准值（毫秒） */
    int retry;       /* 已重试次数 */
};
```

## API

```c
void backoff_init(struct backoff_t * ctx, int delay, int max_delay, int max_retry);
```

初始化退避上下文。`delay` 为初始延迟，`max_delay` 为延迟上限，`max_retry` 为最大重试次数（`<=0` 表示无限制）。`delay` 会被限制在 `[0, max_delay]` 范围内。

```c
int backoff_next(struct backoff_t * ctx);
```

获取下一次重试的等待时间（毫秒）。基于当前 `delay` 计算返回值后再更新 `delay`。达到最大重试次数后返回 -1。

## 增长策略

每次调用 `backoff_next()` 后，`delay` 按以下规则更新：

| 条件 | 更新方式 |
|------|---------|
| `delay < max_delay / 2` | `delay += max(delay, 1)`（当 delay >= 1 时等价于翻倍） |
| `delay >= max_delay / 2` | `delay = max_delay`（封顶） |

其中阈值 `max_delay / 2` 为整数右移计算（`max_delay >> 1`），例如 `max_delay = 1000` 时阈值为 500。

## 演变示例

以 `backoff_init(&bo, 10, 1000, 10)` 为例，`delay` 的演变过程：

| 调用次序 | 当前 delay | 返回范围 | 更新后 delay | 说明 |
|---------|-----------|---------|-------------|------|
| 1 | 10 | [0, 10] | 20 | 10 < 500 → 10+10=20 |
| 2 | 20 | [0, 20] | 40 | 20 < 500 → 20+20=40 |
| 3 | 40 | [0, 40] | 80 | 40 < 500 → 40+40=80 |
| 4 | 80 | [0, 80] | 160 | 80 < 500 → 80+80=160 |
| 5 | 160 | [0, 160] | 320 | 160 < 500 → 160+160=320 |
| 6 | 320 | [0, 320] | 640 | 320 < 500 → 320+320=640 |
| 7 | 640 | [0, 640] | 1000 | 640 >= 500 → 封顶 |
| 8 | 1000 | [0, 1000] | 1000 | 已封顶 |
| 9 | 1000 | [0, 1000] | 1000 | 已封顶 |
| 10 | 1000 | [0, 1000] | 1000 | 已封顶 |
| 11 | - | 返回 -1 | - | 重试耗尽 |

实际运行输出（返回值为随机抖动结果）：

```
[0] next = 1
[1] next = 2
[2] next = 14
[3] next = 30
[4] next = 16
[5] next = 182
[6] next = 265
[7] next = 250
[8] next = 352
[9] next = 308
[10] next = -1
```

## 使用示例

### 网络重连

```c
struct backoff_t bo;
backoff_init(&bo, 100, 30000, 10);

int wait;
while((wait = backoff_next(&bo)) >= 0)
{
    if(try_connect() == SUCCESS)
        break;
    xos_msleep(wait);
}
if(wait < 0)
    /* 重试耗尽 */
```

### 无限重试

```c
struct backoff_t bo;
backoff_init(&bo, 50, 5000, 0);  /* max_retry<=0, 无限制 */

int wait;
while(1)
{
    if(try_connect() == SUCCESS)
        break;
    wait = backoff_next(&bo);
    xos_msleep(wait);
}
```
