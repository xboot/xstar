# Thread Worker (thworker)

An asynchronous task executor based on a background thread, maintaining an internal task queue. The worker thread is woken via a semaphore to execute submitted tasks one by one, suitable for offloading time-consuming operations to a background thread.

## Data Structure

```c
struct thworker_t {
    struct list_head_t head;    /* Task queue */
    struct mutex_t lock;        /* Mutex protecting the task queue */
    struct semaphore_t sem;     /* Semaphore to notify the worker thread of new tasks */
    struct thread_t * thread;   /* Background worker thread */
    int running;                /* Running flag, 1 means running */
};
```

### Task Structure

```c
struct thworker_task_t {
    struct list_head_t entry;   /* List node */
    void (*func)(void *);       /* Task function */
    void * data;                /* User data */
};
```

## How It Works

### Worker Thread Loop

The worker thread is created during `thworker_alloc()` and loops through the following:

1. Wait on the semaphore `sem` (`timeout = 0` means wait indefinitely)
2. After being woken, acquire the lock and dequeue a task from the head of the queue
3. Release the lock and execute the task function
4. Free the task memory and return to step 1

When the `running` flag is set to 0 and the semaphore is posted to wake the thread, the worker thread exits the loop.

### Task Submission

`thworker_submit()` appends a task to the tail of the queue and posts the semaphore to wake the worker thread:

1. Allocate a `thworker_task_t` structure
2. Set the task function and data pointer
3. Acquire the lock and add the task to the tail of the queue
4. Release the lock and `post(&sem)` to wake the worker thread

## API

| Function | Description |
|----------|-------------|
| `thworker_alloc(name)` | Create a worker and start the background thread, `name` is the thread name |
| `thworker_free(w)` | Stop the worker thread and free all resources |
| `thworker_submit(w, func, data)` | Submit an asynchronous task to the work queue |
| `thworker_wait(w)` | Wait for all tasks in the queue to complete |
| `thworker_clear(w, cb)` | Clear pending tasks from the queue, with an optional callback to notify cancelled tasks |

## Usage Examples

### Basic Usage

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

    thworker_wait(w);   /* Wait for all tasks to complete */
    thworker_free(w);   /* Free the worker */
}
```

### Clear with Callback

```c
static void cancel_cb(void (*func)(void *), void * data)
{
    /* Clean up resources when a task is cancelled */
    if(data)
        xos_mem_free(data);
}

void cleanup(struct thworker_t * w)
{
    /* Clear pending tasks, calling the callback for each cancelled task */
    thworker_clear(w, cancel_cb);
    thworker_free(w);
}
```

## Notes

- The worker depends on the thread system; ensure the platform supports threads (`xstar_feature_thread()`) before use
- Tasks are executed in submission order (FIFO)
- `thworker_wait()` polls to check whether the queue is empty, with a 1ms interval between polls
- `thworker_clear()` first clears the pending task queue, then calls `thworker_wait()` to wait for the currently executing task to complete
- `thworker_free()` first calls `thworker_wait()` to ensure the queue is empty, then sets `running = 0` and wakes the worker thread to exit
- The task function must not call `thworker_free()`, as this will cause a deadlock
