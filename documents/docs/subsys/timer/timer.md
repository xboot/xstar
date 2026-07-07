# 定时器 (timer)

基于红黑树的高精度定时器，使用纳秒级 `ktime_t` 时间戳，由时钟事件驱动（clockevent）触发回调，支持周期性和一次性定时。

## 数据结构

### 定时器

```c
struct timer_t {
    struct rb_node node;                          /* 红黑树节点 */
    struct timer_base_t * base;                   /* 所属定时器基 */
    enum timer_state_t state;                     /* 定时器状态 */
    ktime_t expires;                              /* 过期时间 */
    void * data;                                  /* 用户数据 */
    int (*function)(struct timer_t *, void *);    /* 回调函数，返回非 0 表示重新启动 */
};
```

### 定时器基

```c
struct timer_base_t {
    struct rb_root head;       /* 红黑树根节点 */
    struct timer_t * next;     /* 最早过期的定时器 */
    struct clockevent_t * ce;  /* 绑定的时钟事件设备 */
    struct mutex_t lock;       /* 互斥锁，保护红黑树操作 */
};
```

### 状态机

```c
enum timer_state_t {
    TIMER_STATE_INACTIVE = 0,  /* 未激活，不在红黑树中 */
    TIMER_STATE_ENQUEUED = 1,  /* 已入队，等待过期 */
    TIMER_STATE_CALLBACK = 2,  /* 正在执行回调 */
};
```

## 工作原理

### 红黑树管理

所有活跃的定时器按过期时间（`expires`）为键插入红黑树，最早过期的定时器位于最左节点。`base->next` 缓存了最早过期的定时器，避免每次遍历树。

### 时钟事件驱动

定时器系统绑定到一个时钟事件设备（clockevent）：

1. `timer_bind_clockevent()` 绑定时钟事件设备，注册事件处理函数
2. 当有定时器入队时，将最早过期时间编程到时钟事件设备
3. 时钟事件中断触发 `timer_event_handler()`
4. 处理函数从红黑树中取出所有已过期的定时器，执行回调
5. 回调返回非 0 值时，定时器重新入队（实现周期性定时）
6. 处理完毕后，将下一个最早过期时间编程到时钟事件设备

### 线程安全

所有红黑树操作通过互斥锁（`base->lock`）保护，支持在多线程环境下安全使用。回调函数在持锁状态下执行，回调中不可调用 `timer_start()` 或 `timer_cancel()` 操作同一定时器。

## API

| 函数 | 说明 |
|------|------|
| `timer_init(timer, function, data)` | 初始化定时器，设置回调函数和用户数据 |
| `timer_start(timer, interval)` | 启动定时器，`interval` 为相对时间间隔（`ktime_t`），到期后触发回调 |
| `timer_forward(timer, interval)` | 更新定时器过期时间，仅在回调中用于重新设定下次过期时间 |
| `timer_cancel(timer)` | 取消定时器，从红黑树中移除 |
| `timer_bind_clockevent(ce)` | 绑定时钟事件设备，由系统初始化时调用 |

## 用法示例

### 一次性定时器

```c
#include <kernel/time/timer.h>

static int on_timeout(struct timer_t * timer, void * data)
{
    LOG("timeout!\n");
    return 0;  /* 返回 0，不重启 */
}

void demo(void)
{
    struct timer_t timer;
    timer_init(&timer, on_timeout, NULL);
    timer_start(&timer, ms_to_ktime(500));  /* 500ms 后触发 */

    /* ... */

    timer_cancel(&timer);  /* 不再需要时取消 */
}
```

### 周期性定时器

```c
static int on_periodic(struct timer_t * timer, void * data)
{
    int * count = (int *)data;
    LOG("tick: %d\n", (*count)++);
    return 1;  /* 返回非 0，自动重启 */
}

void demo(void)
{
    static struct timer_t timer;
    static int count = 0;

    timer_init(&timer, on_periodic, &count);
    timer_start(&timer, ms_to_ktime(1000));  /* 每 1s 触发 */
}
```

### 回调中调整周期

```c
static int on_variable(struct timer_t * timer, void * data)
{
    LOG("fired\n");
    timer_forward(timer, ms_to_ktime(200));  /* 设定下次 200ms 后触发 */
    return 1;  /* 返回 1，使用 forward 的新时间重新入队 */
}
```

## 说明

- 定时器精度取决于绑定的时钟事件设备，时间戳使用纳秒级 `ktime_t`
- `timer_start()` 可重复调用，会先取消已有定时再重新入队
- `timer_forward()` 仅修改 `expires` 字段，不会重新入队；需配合回调返回 1 使用
- 回调函数返回 1 时，定时器以当前 `expires` 值重新入队；若回调中调用了 `timer_forward()`，则使用更新后的时间
- 回调在持锁状态下执行，执行时间应尽量短，避免影响其他定时器的精度
- 定时器系统通过 `pure_initcall()` 在系统启动早期初始化
