# CoChannel (cochannel)

A ring buffer-based communication channel for coroutines, providing a lock-free data transfer mechanism that works with the coroutine scheduler to implement producer-consumer patterns.

## Data Structure

```c
struct cochannel_t {
    unsigned char * buffer;  /* Ring buffer */
    unsigned int size;       /* Buffer size (power of two) */
    unsigned int in;         /* Write position */
    unsigned int out;        /* Read position */
};
```

## How It Works

### Ring Buffer

The channel uses an internal ring buffer (consistent with the Linux kernel kfifo design):

- **Size Alignment**: Automatically rounded up to the next power of two when allocated, minimum 16 bytes
- **Empty Condition**: `in - out <= 0` means empty
- **Full Condition**: `in - out >= size` means full
- **Index Masking**: Ring wrap-around is achieved via `in & (size - 1)` and `out & (size - 1)`, avoiding modulo operations

### Blocking Send/Receive

`cochannel_send()` and `cochannel_recv()` are blocking interfaces:

- **Send** (`cochannel_send`): Loops to write data; when the buffer is full, calls `coroutine_yield()` to relinquish execution, waiting for the consumer to read before continuing
- **Receive** (`cochannel_recv`): Loops to read data; when the buffer is empty, calls `coroutine_yield()` to relinquish execution, waiting for the producer to write before continuing

Since coroutines use single-threaded cooperative scheduling, ring buffer reads and writes require no locking; synchronization is achieved solely through `yield`.

## API

| Function | Description |
|----------|-------------|
| `cochannel_alloc(size)` | Allocate a channel; `size` is rounded up to the next power of two, minimum 16 bytes |
| `cochannel_free(c)` | Free the channel and its buffer |
| `cochannel_send(sched, c, buf, len)` | Blocking send; automatically yields the coroutine when the buffer is full |
| `cochannel_recv(sched, c, buf, len)` | Blocking receive; automatically yields the coroutine when the buffer is empty |

## Usage Examples

### Producer-Consumer

```c
#include <kernel/core/coroutine.h>
#include <kernel/core/cochannel.h>

static void producer(struct scheduler_t * sched, void * data)
{
    struct cochannel_t * ch = (struct cochannel_t *)data;
    for(int i = 0; i < 5; i++)
    {
        cochannel_send(sched, ch, (unsigned char *)&i, sizeof(i));
        coroutine_msleep(sched, 10);  /* Simulate production time */
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

### Stream Data Transfer

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

## Notes

- Channels depend on the coroutine system; ensure the platform supports coroutines (`xstar_feature_coroutine()`) before use
- The buffer size is automatically rounded up to a power of two; for example, passing 100 allocates 128 bytes
- `cochannel_send`/`cochannel_recv` are blocking and do not busy-wait; they yield the CPU via `yield` when the buffer is unavailable
- Channels are designed for single-producer, single-consumer scenarios; multiple producers or consumers require additional synchronization
- Call `cochannel_free()` to release resources when the channel is no longer needed
