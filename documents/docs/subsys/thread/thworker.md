# 线程工作器 (thworker)

基于后台线程的异步任务执行器，内部维护任务队列，通过信号量唤醒工作线程逐个执行提交的任务，适用于将耗时操作卸载到后台线程的场景。

## 数据结构

```c
struct thworker_t {
    struct list_head_t head;    /* 任务队列 */
    struct mutex_t lock;        /* 互斥锁，保护任务队列 */
    struct semaphore_t sem;     /* 信号量，通知工作线程有新任务 */
    struct thread_t * thread;   /* 后台工作线程 */
    int running;                /* 运行标志，1 表示运行中 */
};
```

### 任务结构

```c
struct thworker_task_t {
    struct list_head_t entry;   /* 链表节点 */
    void (*func)(void *);       /* 任务函数 */
    void * data;                /* 用户数据 */
};
```

## 工作原理

### 工作线程循环

工作线程在 `thworker_alloc()` 时创建，循环执行以下逻辑：

1. 在信号量 `sem` 上等待（`timeout = 0` 表示无限等待）
2. 被唤醒后加锁，从任务队列头部取出一个任务
3. 解锁后执行任务函数
4. 释放任务内存，回到步骤 1

当 `running` 标志被置为 0 且信号量被 `post` 唤醒时，工作线程退出循环。

### 任务提交

`thworker_submit()` 将任务追加到队列尾部并 `post(&sem)` 唤醒工作线程：

1. 分配 `thworker_task_t` 结构体
2. 设置任务函数和数据指针
3. 加锁，将任务加入队列尾部
4. 解锁，`post(&sem)` 唤醒工作线程

## API

| 函数 | 说明 |
|------|------|
| `thworker_alloc(name)` | 创建工作器，启动后台工作线程，`name` 为线程名 |
| `thworker_free(w)` | 停止工作线程并释放所有资源 |
| `thworker_submit(w, func, data)` | 提交异步任务到工作队列 |
| `thworker_wait(w)` | 等待队列中所有任务执行完毕 |
| `thworker_clear(w, cb)` | 清空待执行的任务队列，可通过回调通知被取消的任务 |

## 用法示例

### 基本用法

```c
#include <kernel/core/thworker.h>

static void do_work(void * data)
{
    int * value = (int *)data;
    LOG("processing: %d\n", *value);
    xos_mem_free(value);
}

void demo(void)
{
    struct thworker_t * w = thworker_alloc("my-worker");

    for(int i = 0; i < 5; i++)
    {
        int * v = xos_mem_malloc(sizeof(int));
        *v = i;
        thworker_submit(w, do_work, v);
    }

    thworker_wait(w);   /* 等待所有任务完成 */
    thworker_free(w);   /* 释放工作器 */
}
```

### 带回调的任务清空

```c
static void cancel_cb(void (*func)(void *), void * data)
{
    /* 任务被取消时清理资源 */
    if(data)
        xos_mem_free(data);
}

void cleanup(struct thworker_t * w)
{
    /* 清空未执行的任务，对每个被取消的任务调用回调 */
    thworker_clear(w, cancel_cb);
    thworker_free(w);
}
```

## 说明

- 工作器依赖线程系统，使用前需确保平台支持线程（`xstar_feature_thread()`）
- 任务按提交顺序依次执行（FIFO）
- `thworker_wait()` 通过轮询检查队列是否为空，每次轮询间隔 1ms
- `thworker_clear()` 会先清空待执行的任务队列，再调用 `thworker_wait()` 等待正在执行的任务完成
- `thworker_free()` 会先调用 `thworker_wait()` 确保队列清空，然后设置 `running = 0` 并唤醒工作线程使其退出
- 任务函数内部不可调用 `thworker_free()`，否则会导致死锁
