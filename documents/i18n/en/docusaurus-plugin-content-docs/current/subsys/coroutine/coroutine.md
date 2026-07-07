# Coroutine (coroutine)

A cooperative multitasking coroutine system based on assembly-level context switching, supporting multiple architectures and providing lightweight concurrent execution.

## Data Structures

### Coroutine

```c
struct coroutine_t {
    struct scheduler_t * sched;  /* Owning scheduler */
    struct list_head_t list;     /* Ready list node */
    void * fctx;                 /* Coroutine context (fcontext) */
    void * stack;                /* Coroutine stack space */
    void (*func)(struct scheduler_t *, void *);  /* Coroutine entry function */
    void * data;                 /* User data */
};
```

### Scheduler

```c
struct scheduler_t {
    struct list_head_t ready;    /* Ready coroutine list */
    struct coroutine_t * running;  /* Currently running coroutine */
    void * fctx;                 /* Scheduler context */
};
```

## How It Works

The coroutine system uses cooperative scheduling. Each coroutine voluntarily calls `coroutine_yield()` to relinquish execution, and the scheduler selects the next coroutine from the ready list to run.

### Context Switching

- The underlying implementation uses the XOS abstraction layer's `xos_coroutine_make()` and `xos_coroutine_jump()`
- `xos_coroutine_make()` creates a coroutine context on the specified stack space and sets the entry function
- `xos_coroutine_jump()` performs context switching, passing a `co_transfer_t` structure to save the return context
- Each architecture (ARM32/ARM64/RISC-V/x64) provides its own assembly implementation

### Scheduling Flow

1. `scheduler_init()` initializes the scheduler with an empty ready list
2. `coroutine_start()` creates a coroutine, allocates stack space, and appends it to the ready list
3. `scheduler_loop()` starts scheduling by jumping to the first ready coroutine
4. During execution, a coroutine calls `coroutine_yield()` to relinquish the CPU; the scheduler switches to the next ready coroutine
5. When a coroutine function returns, it is automatically removed from the list and its resources are freed, then the scheduler switches to the next coroutine or returns to the scheduler

## API

| Function | Description |
|----------|-------------|
| `scheduler_init(sched)` | Initialize the scheduler |
| `scheduler_loop(sched)` | Start the scheduling loop, blocks until all coroutines finish |
| `coroutine_start(sched, func, data, stksz)` | Create and start a coroutine, `stksz` is the stack size (default 4096 bytes) |
| `coroutine_yield(sched)` | Relinquish execution, switch to the next ready coroutine |
| `coroutine_msleep(sched, ms)` | Sleep for the specified milliseconds (based on yield polling) |
| `coroutine_usleep(sched, us)` | Sleep for the specified microseconds |
| `coroutine_nsleep(sched, ns)` | Sleep for the specified nanoseconds |
| `coroutine_self(sched)` | Get the currently running coroutine pointer (inline function) |

## Usage Examples

### Basic Coroutine

```c
#include <kernel/core/coroutine.h>

static void worker(struct scheduler_t * sched, void * data)
{
    const char * name = (const char *)data;
    for(int i = 0; i < 3; i++)
    {
        LOG("%s: step %d\n", name, i);
        coroutine_msleep(sched, 100);  /* Sleep 100ms */
    }
}

void demo(void)
{
    struct scheduler_t sched;
    scheduler_init(&sched);

    coroutine_start(&sched, worker, "task-A", 4096);
    coroutine_start(&sched, worker, "task-B", 4096);

    scheduler_loop(&sched);  /* Blocks until all coroutines finish */
}
```

### Coroutine Cooperation

```c
static void producer(struct scheduler_t * sched, void * data)
{
    struct cochannel_t * ch = (struct cochannel_t *)data;
    const char * msg = "hello";
    cochannel_send(sched, ch, (unsigned char *)msg, 5);
}

static void consumer(struct scheduler_t * sched, void * data)
{
    struct cochannel_t * ch = (struct cochannel_t *)data;
    char buf[16] = {0};
    cochannel_recv(sched, ch, (unsigned char *)buf, 5);
    LOG("received: %s\n", buf);
}
```

## Notes

- Coroutines use cooperative scheduling with no preemption; coroutines must voluntarily call `yield` or `sleep` to relinquish execution
- Sleep functions (`msleep`/`usleep`/`nsleep`) are implemented via `ktime_get()` and `yield` polling; precision depends on scheduling frequency
- When a coroutine function returns, its stack space and structure are automatically freed; no manual cleanup is required
- `scheduler_loop()` automatically returns when all coroutines have finished
- Coroutines depend on platform assembly implementations; use `xstar_feature_coroutine()` to check if the current platform supports coroutines
