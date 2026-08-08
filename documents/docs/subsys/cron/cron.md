# Cron 定时任务调度器 (cron)

基于后台线程的墙钟定时任务调度器，支持标准 5 段 cron 表达式。内部维护任务链表，调度线程对齐到分钟边界唤醒，匹配到期任务并执行回调，适用于周期性或一次性的墙钟触发场景。

## 数据结构

```c
struct cron_t {
    struct list_head_t head;    /* 任务链表 */
    struct mutex_t lock;        /* 互斥锁，保护任务链表 */
    struct semaphore_t sem;     /* 信号量，关停时唤醒分钟级睡眠 */
    struct thread_t * thread;   /* 后台调度线程 */
    char * tz;                  /* 时区字符串，透传给 wallclock_gettime */
    int running;                /* 运行标志，1 表示运行中 */
};
```

任务节点 `cron_job_t` 与解析后的表达式 `cron_expr_t` 为内部实现，不对外暴露。

## 工作原理

### 调度线程循环

调度线程在 `cron_alloc()` 时创建，循环执行以下逻辑：

1. 通过 `wallclock_gettime(&tm, tz)` 读取当前墙钟时间
2. 加锁遍历任务链表，对每个未删除的任务进行表达式匹配，命中则记录到快照数组（同时自增引用计数，避免执行期间被释放）
3. 解锁后依次执行命中任务的回调；回调执行完毕后再次加锁，自减引用计数，oneshot 任务此时从链表移除
4. 在信号量 `sem` 上等待至下一个分钟边界（`timeout = (60 - tm.second) * 1000` ms）
5. 回到步骤 1

`cron_free()` 会将 `running` 置 0 并 `post(&sem)` 唤醒睡眠中的线程，使其立即退出循环，而无需等待最长 60 秒的分钟级睡眠。

### 去重与一次性任务

每个任务记录上次触发的"分钟戳"（由年/月/日/时/分打包而成），同一分钟内只触发一次；`oneshot` 任务在回调执行完毕后从链表移除并释放。

### 引用计数

任务命中时引用计数自增，回调执行完毕自减；`cron_remove()` 与 oneshot 自动移除都会先置 `removed` 标志再自减，仅当计数归零时调用 `destroy(data)` 回收 `data` 并释放任务内存。因此在任务回调内部调用 `cron_remove()` 移除自身或其它任务是安全的。

## API

| 函数 | 说明 |
|------|------|
| `cron_alloc(tz)` | 创建调度器并启动后台线程，`tz` 为时区字符串，可为 `NULL` |
| `cron_free(cron)` | 停止线程并释放调度器及所有任务 |
| `cron_add(cron, name, expr, oneshot, exec, destroy, data)` | 添加任务，`name` 需唯一，`oneshot` 非 0 表示仅触发一次；`exec` 为到期回调，`destroy` 为任务销毁时回收 `data` 的回调，可为 `NULL` |
| `cron_remove(cron, name)` | 按名称移除任务 |
| `cron_foreach(cron, cb, data)` | 遍历当前未删除的任务，对每个调用 `cb(name, oneshot, data)` |

返回值：`cron_alloc` 成功返回指针，失败返回 `NULL`；`cron_add`/`cron_remove` 成功返回 `1`，失败返回 `0`。

## 表达式格式

```
*    *    *    *    *
│    │    │    │    │
│    │    │    │    └── day of week (0 - 6, 0 = Sunday)
│    │    │    └─────── month (1 - 12)
│    │    └──────────── day of month (1 - 31)
│    └───────────────── hour (0 - 23)
└────────────────────── minute (0 - 59)
```

表达式由 5 个字段组成，以空格分隔：

| 字段 | 范围 | 说明 |
| --- | --- | --- |
| 分 | 0-59 | 分钟 |
| 时 | 0-23 | 小时，从 0 起计 |
| 日 | 1-31 | 每月第几天，从 1 起计 |
| 月 | 1-12 或 JAN-DEC | 月份，可用名称缩写 |
| 周 | 0-6 或 SUN-SAT | 星期几，0=周日（7 等价于 0）|

每个字段支持以下写法：

| 写法 | 说明 |
| --- | --- |
| `*` | 任意值 |
| `n` | 单个值 |
| `a-b` | 范围 |
| `a-b/s` | 范围内按步进 `s` 取值 |
| `*/s` | 任意值按步进 `s` 取值 |
| `a/s` | 从 `a` 到字段最大值按步进 `s` 取值，如 `5/15` 表示 5,20,35,50 |
| `a,b,c` | 列表，组合上述任意写法 |

月份与星期支持不区分大小写的 3 字母缩写：`JAN`..`DEC`、`SUN`..`SAT`。

## 日与星期

### `7` 等价于 `0`

星期字段中 `7` 是 `0`（周日）的别名，二者等价。因此 `0-7`、`0,7`、`1-7` 都能正确表示"包含周日"。

### 组合规则

遵循 Vixie cron 语义：

- 当**日**和**星期**都被显式指定（均非 `*`）时，命中其中任一即匹配（**或**）
- 否则两者需同时匹配（**与**）

例如 `30 4 1,15 * 5` 表示每月 1 号、15 号或每周五 4:30 执行。

## 用法示例

### 基本用法

```c
#include <kernel/core/cron.h>

static void tick(void * data)
{
    LOG("cron tick\n");
}

void demo(void)
{
    struct cron_t * cron = cron_alloc(NULL);                 /* 默认时区 */
    cron_add(cron, "every-5min", "*/5 * * * *", 0, tick, NULL, NULL);
    /* ... 运行期间持续调度 ... */
    cron_free(cron);
}
```

### 一次性任务

```c
/* 下次 3:30 触发一次后自动从链表移除 */
cron_add(cron, "once", "30 3 * * *", 1, tick, NULL, NULL);
```

### 在回调中移除任务

```c
static void job(void * data)
{
    struct cron_t * cron = (struct cron_t *)data;
    cron_remove(cron, "other-job");   /* 安全：引用计数保护 */
}

void setup(struct cron_t * cron)
{
    cron_add(cron, "self", "*/1 * * * *", 0, job, NULL, cron);
}
```

### 带数据所有权的任务

`data` 通常是为回调准备的堆分配上下文。`cron_add` 成功后其所有权转移给任务，任务销毁时由 `destroy` 回收；添加失败则仍由调用者释放。

```c
static void exec_msg(void * data)
{
    LOG("cron: %s\n", (const char *)data);
}

static void free_msg(void * data)
{
    if(data)
        xos_mem_free(data);
}

void demo(struct cron_t * cron)
{
    char * msg = xos_strdup("hello");
    if(!cron_add(cron, "msg", "*/1 * * * *", 0, exec_msg, free_msg, msg))
        xos_mem_free(msg);   /* 添加失败，调用者自行释放 */
}
```

### 遍历任务

```c
static void list_cb(char * name, int oneshot, void * data)
{
    LOG("job: %s, oneshot=%d\n", name, oneshot);
}

cron_foreach(cron, list_cb, NULL);
```

### 常用表达式

| 表达式 | 含义 |
| --- | --- |
| `*/5 * * * *` | 每 5 分钟 |
| `0 * * * *` | 每小时整点 |
| `30 3 * * *` | 每天 3:30 |
| `0 9 * * 1` | 每周一 9:00 |
| `0 0 1 * *` | 每月 1 号 0:00 |
| `0 0 * * 5` | 每周五 0:00 |
| `0 0 * * 0` | 每周日 0:00（`7` 同义）|
| `0 0 1 1 *` | 每年 1 月 1 号 0:00 |
| `30 4 1,15 * 5` | 每月 1、15 号或每周五 4:30 |
| `*/15 * * * *` | 每 15 分钟 |

## 说明

- 调度器依赖线程系统，使用前需确保平台支持线程（`xstar_feature_thread()`），否则 `cron_alloc()` 返回 `NULL`
- 任务 `name` 必须唯一，重名添加会失败
- `cron_add` 成功后 `data` 的所有权转移给任务：任务销毁时（oneshot 触发后自动移除、`cron_remove` 或 `cron_free`）调用 `destroy(data)` 回收；`destroy` 为 `NULL` 时 cron 不触碰 `data`，由调用者自行管理其生命周期
- `cron_add` 失败时所有权未转移，`data` 仍由调用者释放
- 时间源为墙钟 `wallclock_gettime()`，`tz` 直接透传；`wallclock_time_t.week` 定义为 `(自 1970-01-01 起的天数 + 4) % 7`，`0 = 周日`，与 cron 标准一致
- 调度精度为分钟级，每分钟边界检查一次；若某分钟被跳过（如系统繁忙），该分钟的任务不会补触发
- `cron_add`/`cron_remove`/`cron_foreach` 可在任意线程调用，包括任务回调内部
- `cron_free` 会唤醒并等待调度线程退出后再释放所有任务，调用后不可再使用该句柄
