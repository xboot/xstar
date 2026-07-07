# 线程通道 (thchannel)

基于环形缓冲区的线程间通信通道，使用互斥锁和信号量实现线程安全的数据传输，支持超时机制，适用于多线程环境下的生产者-消费者场景。

## 数据结构

```c
struct thchannel_t {
    unsigned char * buffer;     /* 环形缓冲区 */
    unsigned int size;          /* 缓冲区大小（2 的幂次方） */
    unsigned int in;            /* 写入位置 */
    unsigned int out;           /* 读取位置 */
    struct mutex_t lock;        /* 互斥锁，保护缓冲区操作 */
    struct semaphore_t ssem;    /* 发送信号量，缓冲区有空闲空间时通知发送方 */
    struct semaphore_t rsem;    /* 接收信号量，缓冲区有数据时通知接收方 */
};
```

## 工作原理

### 环形缓冲区

与协程通道（cochannel）设计一致，采用 Linux kernel kfifo 风格的环形缓冲区：

- **大小对齐**：分配时自动向上取整为 2 的幂次方，最小 16 字节
- **判空条件**：`in - out == 0`
- **判满条件**：`in - out == size`
- **索引掩码**：通过 `in & (size - 1)` 和 `out & (size - 1)` 实现环形回绕

### 线程同步

与协程通道不同，线程通道使用互斥锁和信号量实现真正的多线程同步：

- **互斥锁**（`lock`）：保护 `in`/`out` 指针和缓冲区数据的原子访问
- **发送信号量**（`ssem`）：缓冲区满时，发送方在 `ssem` 上等待；接收方读取数据后 `post(&ssem)` 唤醒发送方
- **接收信号量**（`rsem`）：缓冲区空时，接收方在 `rsem` 上等待；发送方写入数据后 `post(&rsem)` 唤醒接收方

### 超时机制

`thchannel_send()` 和 `thchannel_recv()` 支持超时参数：

- `timeout = 0`：非阻塞模式，无数据可读写时立即返回
- `timeout > 0`：阻塞等待指定毫秒数，超时后返回已传输的数据量
- 返回值为实际发送/接收的字节数

## API

| 函数 | 说明 |
|------|------|
| `thchannel_alloc(size)` | 分配通道，`size` 自动取整为 2 的幂次方，最小 16 字节 |
| `thchannel_free(c)` | 释放通道及其缓冲区、互斥锁和信号量 |
| `thchannel_reset(c)` | 重置通道，清空缓冲区（`in = out = 0`） |
| `thchannel_isempty(c)` | 判断通道是否为空 |
| `thchannel_isfull(c)` | 判断通道是否已满 |
| `thchannel_size(c)` | 获取通道总容量 |
| `thchannel_length(c)` | 获取当前已缓冲的数据长度 |
| `thchannel_available(c)` | 获取剩余可用空间 |
| `thchannel_send(c, buf, len, timeout)` | 阻塞式发送数据，支持超时，返回实际发送字节数 |
| `thchannel_recv(c, buf, len, timeout)` | 阻塞式接收数据，支持超时，返回实际接收字节数 |

## 用法示例

### 生产者-消费者

```c
#include <kernel/core/thchannel.h>

static struct thchannel_t * ch;

static void producer(void * data)
{
    for(int i = 0; i < 10; i++)
    {
        thchannel_send(ch, (unsigned char *)&i, sizeof(i), 1000);
        xos_thread_msleep(10);
    }
}

static void consumer(void * data)
{
    for(int i = 0; i < 10; i++)
    {
        int value;
        thchannel_recv(ch, (unsigned char *)&value, sizeof(value), 1000);
        LOG("received: %d\n", value);
    }
}

void demo(void)
{
    ch = thchannel_alloc(256);

    struct thread_t * tp = xos_thread_create("producer", producer, NULL, 0);
    struct thread_t * tc = xos_thread_create("consumer", consumer, NULL, 0);

    xos_thread_wait(tp);
    xos_thread_wait(tc);
    xos_thread_destroy(tp);
    xos_thread_destroy(tc);

    thchannel_free(ch);
}
```

### 非阻塞式读取

```c
/* timeout = 0，非阻塞模式 */
char buf[64];
unsigned int len = thchannel_recv(ch, (unsigned char *)buf, sizeof(buf), 0);
if(len > 0)
{
    LOG("got %u bytes\n", len);
}
```

## 说明

- 通道依赖线程系统，使用前需确保平台支持线程（`xstar_feature_thread()`）
- 通道线程安全，支持多生产者、多消费者场景
- 缓冲区大小自动取整为 2 的幂次方，例如传入 100 则实际分配 128 字节
- `thchannel_reset()` 仅清空缓冲区，不影响正在等待的信号量；如需安全重置，应确保没有线程在等待
- 通道使用完毕后需调用 `thchannel_free()` 释放资源
