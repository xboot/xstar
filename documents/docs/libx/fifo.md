# 环形缓冲区 (fifo)

无锁环形缓冲区 FIFO，提供两套 API：无锁 `__fifo_*`（调用者自行同步）和加锁 `fifo_*`（内部互斥锁）。支持原始字节流和带长度前缀的数据包。

## API

```c
struct fifo_t * fifo_alloc(unsigned int size);
void fifo_free(struct fifo_t * f);
void fifo_reset(struct fifo_t * f);
int fifo_isempty(struct fifo_t * f);
int fifo_isfull(struct fifo_t * f);
unsigned int fifo_size(struct fifo_t * f);
unsigned int fifo_length(struct fifo_t * f);
unsigned int fifo_available(struct fifo_t * f);
unsigned int fifo_put(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int fifo_get(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int fifo_peek(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int fifo_put_packet(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int fifo_get_packet(struct fifo_t * f, unsigned char * buf, unsigned int len);
```

- `fifo_alloc` — 分配 FIFO（size 自动向上取整为 2 的幂）
- `fifo_free` — 释放
- `fifo_reset` — 重置
- `fifo_isempty` / `fifo_isfull` — 判空/判满
- `fifo_size` / `fifo_length` / `fifo_available` — 容量/已用/可用
- `fifo_put` / `fifo_get` — 写入/读取原始字节
- `fifo_peek` — 窥探数据，不移动读指针
- `fifo_put_packet` / `fifo_get_packet` — 写入/读取带长度前缀的数据包

## 使用示例

```c
struct fifo_t * f = fifo_alloc(1024);

unsigned char data[] = { 0x01, 0x02, 0x03 };
fifo_put(f, data, sizeof(data));

unsigned char buf[16];
unsigned int n = fifo_get(f, buf, sizeof(buf));

fifo_put_packet(f, data, sizeof(data));
n = fifo_get_packet(f, buf, sizeof(buf));

fifo_free(f);
```

## Packet 模式说明

`fifo_put_packet` 写入时自动添加 4 字节长度前缀，`fifo_get_packet` 读取时自动解析。零长度包会被拒绝（`len` 必须大于 0）。消费者 buf 不足时，整包仍会被消费，仅拷贝 `len` 字节并返回实际拷贝长度。可通过 `fifo_peek` 先获取包长，再准备足够的缓冲区：

```c
unsigned int pkt_len;
if(fifo_peek(f, (unsigned char *)&pkt_len, sizeof(unsigned int)) == sizeof(unsigned int))
{
    unsigned char buf[pkt_len];
    fifo_get_packet(f, buf, pkt_len);
}
```
