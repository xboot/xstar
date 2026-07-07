# Thread Channel (thchannel)

A ring buffer-based inter-thread communication channel that uses mutexes and semaphores to achieve thread-safe data transfer with timeout support, suitable for producer-consumer scenarios in multi-threaded environments.

## Data Structure

```c
struct thchannel_t {
    unsigned char * buffer;     /* Ring buffer */
    unsigned int size;          /* Buffer size (power of two) */
    unsigned int in;            /* Write position */
    unsigned int out;           /* Read position */
    struct mutex_t lock;        /* Mutex protecting buffer operations */
    struct semaphore_t ssem;    /* Sender semaphore, notifies sender when space is available */
    struct semaphore_t rsem;    /* Receiver semaphore, notifies receiver when data is available */
};
```

## How It Works

### Ring Buffer

Consistent with the cochannel design, it uses a Linux kernel kfifo-style ring buffer:

- **Size Alignment**: Automatically rounded up to the next power of two when allocated, minimum 16 bytes
- **Empty Condition**: `in - out == 0`
- **Full Condition**: `in - out == size`
- **Index Masking**: Ring wrap-around is achieved via `in & (size - 1)` and `out & (size - 1)`

### Thread Synchronization

Unlike the cochannel, the thread channel uses mutexes and semaphores for true multi-threaded synchronization:

- **Mutex** (`lock`): Protects atomic access to the `in`/`out` pointers and buffer data
- **Sender Semaphore** (`ssem`): When the buffer is full, the sender waits on `ssem`; the receiver calls `post(&ssem)` after reading data to wake up the sender
- **Receiver Semaphore** (`rsem`): When the buffer is empty, the receiver waits on `rsem`; the sender calls `post(&rsem)` after writing data to wake up the receiver

### Timeout Mechanism

`thchannel_send()` and `thchannel_recv()` support a timeout parameter:

- `timeout = 0`: Non-blocking mode, returns immediately when no data can be read or written
- `timeout > 0`: Blocks for the specified milliseconds, returns the amount of data transferred after timeout
- Return value is the actual number of bytes sent/received

## API

| Function | Description |
|----------|-------------|
| `thchannel_alloc(size)` | Allocate a channel; `size` is rounded up to the next power of two, minimum 16 bytes |
| `thchannel_free(c)` | Free the channel along with its buffer, mutex, and semaphores |
| `thchannel_reset(c)` | Reset the channel, clear the buffer (`in = out = 0`) |
| `thchannel_isempty(c)` | Check if the channel is empty |
| `thchannel_isfull(c)` | Check if the channel is full |
| `thchannel_size(c)` | Get the total capacity of the channel |
| `thchannel_length(c)` | Get the current buffered data length |
| `thchannel_available(c)` | Get the remaining available space |
| `thchannel_send(c, buf, len, timeout)` | Blocking send with timeout support, returns actual bytes sent |
| `thchannel_recv(c, buf, len, timeout)` | Blocking receive with timeout support, returns actual bytes received |

## Usage Examples

### Producer-Consumer

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

### Non-blocking Read

```c
/* timeout = 0, non-blocking mode */
char buf[64];
unsigned int len = thchannel_recv(ch, (unsigned char *)buf, sizeof(buf), 0);
if(len > 0)
{
    LOG("got %u bytes\n", len);
}
```

## Notes

- Channels depend on the thread system; ensure the platform supports threads (`xstar_feature_thread()`) before use
- Channels are thread-safe and support multi-producer, multi-consumer scenarios
- The buffer size is automatically rounded up to a power of two; for example, passing 100 allocates 128 bytes
- `thchannel_reset()` only clears the buffer and does not affect waiting semaphores; to safely reset, ensure no threads are waiting
- Call `thchannel_free()` to release resources when the channel is no longer needed
