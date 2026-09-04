# 线程邮箱 (thmailbox)

基于定长块内存池的零拷贝线程间消息邮箱。消息在环形队列中只传递指针而不拷贝数据，配合缓冲区所有权转移机制，适用于音频、显示等大块数据的流水线式生产者-消费者场景。

## 数据结构

```c
struct thmailbox_t {
    unsigned char * pool;       /* 定长块内存池 */
    unsigned int block_size;    /* 每块负载容量 */
    unsigned int block_count;   /* 块数量 */
    void * freelist;            /* 空闲块链表 */
    unsigned int freecount;     /* 空闲块计数 */
    void ** ring;               /* 消息描述符环形队列 */
    unsigned int ring_size;     /* 队列长度（2 的幂次方） */
    unsigned int in;            /* 写入位置 */
    unsigned int out;           /* 读取位置 */
    struct mutex_t lock;        /* 互斥锁，保护池和队列操作 */
    struct semaphore_t ssem;    /* 发送信号量，队列有空位时通知发送方 */
    struct semaphore_t rsem;    /* 接收信号量，队列有消息时通知接收方 */
    struct semaphore_t psem;    /* 池信号量，有块归还时通知等待分配的线程 */
};
```

## 工作原理

### 定长块内存池

邮箱在创建时一次性分配 `count` 个 `[块头][负载]` 布局的定长块：

- **块头隐藏**：每块负载前有一个内部 `bufhdr_t` 头（魔数、消息长度、所属邮箱指针），调用方拿到的永远是纯负载指针
- **空闲链表**：空闲块通过块头的 `next` 字段串成单向链表，不占用额外内存；块归还时魔数被 `next` 覆盖，天然识别非法的双重释放
- **提前校验**：`thmailbox_buf_alloc()` 在写入数据前就检查 `len <= block_size`，消息过长立即返回 `NULL`

### 零拷贝与所有权转移

`thchannel` 逐字节拷贝数据，而邮箱只搬运指针：

1. 发送方 `thmailbox_buf_alloc()` 从池中取块（所有权归属发送方）
2. 直接写入块内，`thmailbox_send()` 将 `{指针, 长度}` 描述符入队，所有权移交给接收方
3. 接收方 `thmailbox_recv()` 取出指针直接读取，用完后 `thmailbox_buf_free()` 归还池

发送后发送方不得再访问该块；接收方用完必须归还，否则池逐渐耗尽。

### 线程同步

- **互斥锁**（`lock`）：保护空闲链表、`in`/`out` 指针和描述符队列的原子访问
- **发送信号量**（`ssem`）：队列满时发送方等待；接收方取走消息后唤醒
- **接收信号量**（`rsem`）：队列空时接收方等待；发送方投递消息后唤醒
- **池信号量**（`psem`）：池耗尽时 `thmailbox_buf_alloc()` 阻塞等待；块被归还时唤醒——这为生产方提供了天然背压

### 超时机制

`thmailbox_buf_alloc()`、`thmailbox_send()` 和 `thmailbox_recv()` 均支持超时参数：

- `timeout < 0`：永久阻塞
- `timeout = 0`：非阻塞模式，立即返回
- `timeout > 0`：阻塞等待指定毫秒数

`send`/`recv` 返回消息长度（0 表示失败或超时），因此消息长度不允许为 0。

## API

| 函数 | 说明 |
|------|------|
| `thmailbox_alloc(size, count)` | 分配邮箱，`size` 为单块负载容量，`count` 为块数量（同时决定队列深度，自动取整为 2 的幂次方） |
| `thmailbox_free(m)` | 释放邮箱及其内存池、队列、互斥锁和信号量 |
| `thmailbox_reset(m)` | 重置邮箱，将队列中的消息按所有权规则归还池并清空 |
| `thmailbox_buf_alloc(m, len, timeout)` | 从池中分配一块缓冲区，支持超时，池耗尽返回 `NULL` |
| `thmailbox_buf_free(m, buf)` | 将缓冲区归还池（接收方消费完毕后调用） |
| `thmailbox_buf_available(m)` | 获取池中空闲块数量 |
| `thmailbox_send(m, buf, len, timeout)` | 阻塞式投递消息（移交 `buf` 所有权），返回消息长度 |
| `thmailbox_recv(m, &pbuf, timeout)` | 阻塞式接收消息，数据指针写入 `pbuf`，返回消息长度 |

## 用法示例

### 零拷贝生产者-消费者

```c
#include <kernel/core/thmailbox.h>

static struct thmailbox_t * mb;

static void producer(void * data)
{
    for(int i = 0; i < 10; i++)
    {
        /* 池耗尽时阻塞等待（背压），而非丢数据 */
        unsigned char * buf = thmailbox_buf_alloc(mb, 256, -1);
        if(buf)
        {
            int len = fill_frame(buf, 256, i);
            thmailbox_send(mb, buf, len, -1);   /* 之后 buf 不再属于发送方 */
        }
    }
}

static void consumer(void * data)
{
    for(int i = 0; i < 10; i++)
    {
        void * buf;
        unsigned int len = thmailbox_recv(mb, &buf, -1);
        if(len > 0)
        {
            process_frame(buf, len);
            thmailbox_buf_free(mb, buf);        /* 用完归还，否则池会耗尽 */
        }
    }
}

void demo(void)
{
    mb = thmailbox_alloc(256, 8);   /* 8 块 × 256 字节，队列深度 8 */

    struct thread_t * tp = xos_thread_create("producer", producer, NULL, 0);
    struct thread_t * tc = xos_thread_create("consumer", consumer, NULL, 0);

    xos_thread_wait(tp);
    xos_thread_wait(tc);
    xos_thread_destroy(tp);
    xos_thread_destroy(tc);

    thmailbox_free(mb);
}
```

### 池压力感知（丢帧策略）

```c
/* 空闲块不足时直接丢弃，不阻塞 */
if(thmailbox_buf_available(mb) > 2)
{
    unsigned char * buf = thmailbox_buf_alloc(mb, 256, 0);
    if(buf)
        thmailbox_send(mb, buf, fill_frame(buf, 256), 0);
}
else
{
    drop_frame();
}
```

## 说明

- 邮箱依赖线程系统，使用前需确保平台支持线程（`xstar_feature_thread()`）
- 邮箱线程安全，支持多生产者、多消费者场景
- `size` 由流经管道的最大单条消息决定（向上取整到 2 的幂或 cache line 倍数）；`count` 由期望的流水线深度决定（生产速率 × 可容忍停顿时间 + 余量）
- `send` 要求 `1 <= len <= size`；`recv` 返回 0 表示超时或失败，因此不允许零长度消息
- 传入 `thmailbox_send()` 的指针必须来自同一邮箱的 `thmailbox_buf_alloc()`，外部指针会被魔数与归属校验拒绝
- `thmailbox_free()` 前需确保所有在途缓冲区已归还、没有线程阻塞等待；`thmailbox_reset()` 仅回收队列内的消息，接收方手中持有的块不受影响
