# 协程通道 (cochannel)

基于环形缓冲区的协程间通信通道，提供无锁的数据传输机制，配合协程调度器实现生产者-消费者模式。

## 数据结构

```c
struct cochannel_t {
    unsigned char * buffer;  /* 环形缓冲区 */
    unsigned int size;       /* 缓冲区大小（2 的幂次方） */
    unsigned int in;         /* 写入位置 */
    unsigned int out;        /* 读取位置 */
};
```

## 工作原理

### 环形缓冲区

通道内部使用环形缓冲区（与 Linux kernel kfifo 设计一致）：

- **大小对齐**：分配时自动向上取整为 2 的幂次方，最小 16 字节
- **判空条件**：`in - out <= 0` 时为空
- **判满条件**：`in - out >= size` 时为满
- **索引掩码**：通过 `in & (size - 1)` 和 `out & (size - 1)` 实现环形回绕，无需取模运算

### 阻塞式收发

`cochannel_send()` 和 `cochannel_recv()` 为阻塞式接口：

- **发送**（`cochannel_send`）：循环写入数据，当缓冲区满时调用 `coroutine_yield()` 让出执行权，等待消费者读取后再继续
- **接收**（`cochannel_recv`）：循环读取数据，当缓冲区空时调用 `coroutine_yield()` 让出执行权，等待生产者写入后再继续

由于协程为单线程协作式调度，环形缓冲区的读写无需加锁，仅通过 `yield` 实现同步。

## API

| 函数 | 说明 |
|------|------|
| `cochannel_alloc(size)` | 分配通道，`size` 自动向上取整为 2 的幂次方，最小 16 字节 |
| `cochannel_free(c)` | 释放通道及其缓冲区 |
| `cochannel_send(sched, c, buf, len)` | 阻塞式发送数据，缓冲区满时自动让出协程 |
| `cochannel_recv(sched, c, buf, len)` | 阻塞式接收数据，缓冲区空时自动让出协程 |

## 用法示例

### 生产者-消费者

```c
#include <kernel/core/coroutine.h>
#include <kernel/core/cochannel.h>

static void producer(struct scheduler_t * sched, void * data)
{
    struct cochannel_t * ch = (struct cochannel_t *)data;
    for(int i = 0; i < 5; i++)
    {
        cochannel_send(sched, ch, (unsigned char *)&i, sizeof(i));
        coroutine_msleep(sched, 10);  /* 模拟生产耗时 */
    }
}

static void consumer(struct scheduler_t * sched, void * data)
{
    struct cochannel_t * ch = (struct cochannel_t *)data;
    for(int i = 0; i < 5; i++)
    {
        int value;
        cochannel_recv(sched, ch, (unsigned char *)&value, sizeof(value));
        LOG("consumed: %d\n", value);
    }
}

void demo(void)
{
    struct scheduler_t sched;
    scheduler_init(&sched);

    struct cochannel_t * ch = cochannel_alloc(64);

    coroutine_start(&sched, producer, ch, 4096);
    coroutine_start(&sched, consumer, ch, 4096);

    scheduler_loop(&sched);
    cochannel_free(ch);
}
```

### 流式数据传输

```c
static void sender(struct scheduler_t * sched, void * data)
{
    struct cochannel_t * ch = (struct cochannel_t *)data;
    char * text = "Hello, Coroutine!";
    cochannel_send(sched, ch, (unsigned char *)text, strlen(text));
}

static void receiver(struct scheduler_t * sched, void * data)
{
    struct cochannel_t * ch = (struct cochannel_t *)data;
    char buf[32] = {0};
    cochannel_recv(sched, ch, (unsigned char *)buf, 17);
    LOG("got: %s\n", buf);
}
```

## 说明

- 通道依赖协程系统，使用前需确保平台支持协程（`xstar_feature_coroutine()`）
- 缓冲区大小自动取整为 2 的幂次方，例如传入 100 则实际分配 128 字节
- `cochannel_send`/`cochannel_recv` 为阻塞式，不会忙等待，缓冲区不可操作时通过 `yield` 让出 CPU
- 通道为单生产者-单消费者场景设计，多生产者或多消费者需额外同步
- 通道使用完毕后需调用 `cochannel_free()` 释放资源
