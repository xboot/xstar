# Thread Mailbox (thmailbox)

A zero-copy inter-thread message mailbox based on a fixed-size block pool. Messages cross the ring queue as pointers instead of data copies, combined with buffer ownership transfer, making it suitable for pipelined producer-consumer scenarios with large payloads such as audio and display data.

## Data Structure

```c
struct thmailbox_t {
    unsigned char * pool;       /* Fixed-size block pool */
    unsigned int block_size;    /* Payload capacity per block */
    unsigned int block_count;   /* Number of blocks */
    void * freelist;            /* Free block list */
    unsigned int freecount;     /* Free block count */
    void ** ring;               /* Message descriptor ring queue */
    unsigned int ring_size;     /* Queue length (power of two) */
    unsigned int in;            /* Write position */
    unsigned int out;           /* Read position */
    struct mutex_t lock;        /* Mutex protecting pool and queue operations */
    struct semaphore_t ssem;    /* Sender semaphore, notifies sender when a queue slot is free */
    struct semaphore_t rsem;    /* Receiver semaphore, notifies receiver when a message arrives */
    struct semaphore_t psem;    /* Pool semaphore, notifies allocators when a block is recycled */
};
```

## How It Works

### Fixed-Size Block Pool

The mailbox allocates `count` fixed-size blocks in one shot at creation, each laid out as `[block header][payload]`:

- **Hidden header**: Each payload is preceded by an internal `bufhdr_t` header (magic, message length, owning mailbox pointer); callers always receive a pure payload pointer
- **Free list**: Free blocks are chained into a singly linked list through the header's `next` field, consuming no extra memory; when a block is recycled, the magic is overwritten by `next`, which naturally rejects illegal double frees
- **Early validation**: `thmailbox_buf_alloc()` checks `len <= block_size` before any data is written; oversized requests return `NULL` immediately

### Zero Copy and Ownership Transfer

While `thchannel` copies data byte by byte, the mailbox only moves pointers:

1. The sender calls `thmailbox_buf_alloc()` to take a block from the pool (ownership belongs to the sender)
2. The sender writes directly into the block, then `thmailbox_send()` enqueues a `{pointer, length}` descriptor and ownership moves to the receiver
3. The receiver takes the pointer via `thmailbox_recv()` and reads it in place, then returns the block with `thmailbox_buf_free()`

After sending, the sender must not access the block again; the receiver must return every consumed block, otherwise the pool drains gradually.

### Thread Synchronization

- **Mutex** (`lock`): Protects atomic access to the free list, the `in`/`out` pointers, and the descriptor queue
- **Sender semaphore** (`ssem`): The sender waits when the queue is full; the receiver wakes it after taking a message
- **Receiver semaphore** (`rsem`): The receiver waits when the queue is empty; the sender wakes it after posting a message
- **Pool semaphore** (`psem`): `thmailbox_buf_alloc()` blocks when the pool is exhausted and is woken when a block is recycled — providing natural backpressure for producers

### Timeout Mechanism

`thmailbox_buf_alloc()`, `thmailbox_send()`, and `thmailbox_recv()` all support a timeout parameter:

- `timeout < 0`: Blocks forever
- `timeout = 0`: Non-blocking mode, returns immediately
- `timeout > 0`: Blocks for the specified milliseconds

`send`/`recv` return the message length (0 means failure or timeout), therefore zero-length messages are not allowed.

## API

| Function | Description |
|----------|-------------|
| `thmailbox_alloc(size, count)` | Allocate a mailbox; `size` is the payload capacity per block, `count` is the number of blocks (also determines queue depth, rounded up to a power of two) |
| `thmailbox_free(m)` | Free the mailbox along with its pool, queue, mutex, and semaphores |
| `thmailbox_reset(m)` | Reset the mailbox, returning queued messages to the pool per ownership rules and clearing the queue |
| `thmailbox_buf_alloc(m, len, timeout)` | Allocate a buffer from the pool with timeout support, returns `NULL` when exhausted |
| `thmailbox_buf_free(m, buf)` | Return a buffer to the pool (called by the consumer after use) |
| `thmailbox_buf_available(m)` | Get the number of free blocks in the pool |
| `thmailbox_send(m, buf, len, timeout)` | Blocking send (transfers ownership of `buf`), returns the message length |
| `thmailbox_recv(m, &pbuf, timeout)` | Blocking receive, writes the data pointer into `pbuf`, returns the message length |

## Usage Examples

### Zero-Copy Producer-Consumer

```c
#include <kernel/core/thmailbox.h>

static struct thmailbox_t * mb;

static void producer(void * data)
{
    for(int i = 0; i < 10; i++)
    {
        /* Blocks when the pool is exhausted (backpressure) instead of dropping data */
        unsigned char * buf = thmailbox_buf_alloc(mb, 256, -1);
        if(buf)
        {
            int len = fill_frame(buf, 256, i);
            thmailbox_send(mb, buf, len, -1);   /* buf no longer belongs to the sender */
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
            thmailbox_buf_free(mb, buf);        /* Return after use, or the pool drains */
        }
    }
}

void demo(void)
{
    mb = thmailbox_alloc(256, 8);   /* 8 blocks x 256 bytes, queue depth 8 */

    struct thread_t * tp = xos_thread_create("producer", producer, NULL, 0);
    struct thread_t * tc = xos_thread_create("consumer", consumer, NULL, 0);

    xos_thread_wait(tp);
    xos_thread_wait(tc);
    xos_thread_destroy(tp);
    xos_thread_destroy(tc);

    thmailbox_free(mb);
}
```

### Pool Pressure Awareness (Drop Policy)

```c
/* Drop when free blocks run low, without blocking */
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

## Notes

- Mailboxes depend on the thread system; ensure the platform supports threads (`xstar_feature_thread()`) before use
- Mailboxes are thread-safe and support multi-producer, multi-consumer scenarios
- `size` is determined by the largest single message flowing through the pipeline (rounded up to a power of two or a multiple of the cache line); `count` is determined by the desired pipeline depth (production rate x tolerable stall time + margin)
- `send` requires `1 <= len <= size`; `recv` returns 0 on timeout or failure, so zero-length messages are not allowed
- Pointers passed to `thmailbox_send()` must originate from `thmailbox_buf_alloc()` of the same mailbox; foreign pointers are rejected by the magic and ownership checks
- Before calling `thmailbox_free()`, ensure all in-flight buffers have been returned and no threads are blocked waiting; `thmailbox_reset()` only recycles messages still in the queue and does not affect blocks held by receivers
