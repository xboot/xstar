# 协程 (coroutine)

基于汇编级上下文切换的协作式多任务协程系统，支持多种架构，提供轻量级的并发执行能力。

## 数据结构

### 协程

```c
struct coroutine_t {
    struct scheduler_t * sched;  /* 所属调度器 */
    struct list_head_t list;     /* 就绪链表节点 */
    void * fctx;                 /* 协程上下文（fcontext） */
    void * stack;                /* 协程栈空间 */
    void (*func)(struct scheduler_t *, void *);  /* 协程入口函数 */
    void * data;                 /* 用户数据 */
};
```

### 调度器

```c
struct scheduler_t {
    struct list_head_t ready;    /* 就绪协程链表 */
    struct coroutine_t * running;  /* 当前运行的协程 */
    void * fctx;                 /* 调度器上下文 */
};
```

## 工作原理

协程系统采用协作式调度，每个协程主动调用 `coroutine_yield()` 让出执行权，调度器从就绪链表中选择下一个协程运行。

### 上下文切换

- 底层通过 XOS 抽象层的 `xos_coroutine_make()` 和 `xos_coroutine_jump()` 实现
- `xos_coroutine_make()` 在指定栈空间上创建协程上下文，设置入口函数
- `xos_coroutine_jump()` 执行上下文切换，传递 `co_transfer_t` 结构体保存返回上下文
- 每种架构（ARM32/ARM64/RISC-V/x64）提供各自的汇编实现

### 调度流程

1. `scheduler_init()` 初始化调度器，就绪链表为空
2. `coroutine_start()` 创建协程，分配栈空间，加入就绪链表尾部
3. `scheduler_loop()` 启动调度，跳转到第一个就绪协程执行
4. 协程执行过程中调用 `coroutine_yield()` 让出 CPU，调度器切换到下一个就绪协程
5. 协程函数返回后，自动从链表移除并释放资源，切换到下一个协程或返回调度器

## API

| 函数 | 说明 |
|------|------|
| `scheduler_init(sched)` | 初始化调度器 |
| `scheduler_loop(sched)` | 启动调度循环，阻塞直到所有协程结束 |
| `coroutine_start(sched, func, data, stksz)` | 创建并启动协程，`stksz` 为栈大小（默认 4096 字节） |
| `coroutine_yield(sched)` | 让出执行权，切换到下一个就绪协程 |
| `coroutine_msleep(sched, ms)` | 协程休眠指定毫秒数（基于 yield 轮询） |
| `coroutine_usleep(sched, us)` | 协程休眠指定微秒数 |
| `coroutine_nsleep(sched, ns)` | 协程休眠指定纳秒数 |
| `coroutine_self(sched)` | 获取当前运行的协程指针（内联函数） |

## 用法示例

### 基本协程

```c
#include <kernel/core/coroutine.h>

static void worker(struct scheduler_t * sched, void * data)
{
    const char * name = (const char *)data;
    for(int i = 0; i < 3; i++)
    {
        LOG("%s: step %d\n", name, i);
        coroutine_msleep(sched, 100);  /* 休眠 100ms */
    }
}

void demo(void)
{
    struct scheduler_t sched;
    scheduler_init(&sched);

    coroutine_start(&sched, worker, "task-A", 4096);
    coroutine_start(&sched, worker, "task-B", 4096);

    scheduler_loop(&sched);  /* 阻塞直到所有协程结束 */
}
```

### 协程间协作

```c
static void producer(struct scheduler_t * sched, void * data)
{
    struct cochannel_t * ch = (struct cochannel_t *)data;
    const char * msg = "hello";
    cochannel_send(sched, ch, (unsigned char *)msg, 5);
}

static void consumer(struct scheduler_t * sched, void * data)
{
    struct cochannel_t * ch = (struct cochannel_t *)data;
    char buf[16] = {0};
    cochannel_recv(sched, ch, (unsigned char *)buf, 5);
    LOG("received: %s\n", buf);
}
```

## 说明

- 协程为协作式调度，不会抢占，必须主动调用 `yield` 或 `sleep` 让出执行权
- 休眠函数（`msleep`/`usleep`/`nsleep`）基于 `ktime_get()` 和 `yield` 轮询实现，精度取决于调度频率
- 协程函数返回后自动释放栈空间和协程结构体，无需手动清理
- `scheduler_loop()` 在所有协程结束后自动返回
- 协程依赖平台汇编实现，通过 `xstar_feature_coroutine()` 可检测当前平台是否支持
