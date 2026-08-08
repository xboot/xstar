# Cron Job Scheduler (cron)

A wall-clock based job scheduler running on a background thread, supporting standard 5-field cron expressions. It maintains an internal job list; the scheduler thread wakes up aligned to the minute boundary, matches due jobs, and invokes their callbacks, suitable for periodic or one-shot wall-clock triggered tasks.

## Data Structure

```c
struct cron_t {
    struct list_head_t head;    /* Job list */
    struct mutex_t lock;        /* Mutex protecting the job list */
    struct semaphore_t sem;     /* Semaphore to wake the minute-level sleep on shutdown */
    struct thread_t * thread;   /* Background scheduler thread */
    char * tz;                  /* Timezone string, forwarded to wallclock_gettime */
    int running;                /* Running flag, 1 means running */
};
```

The job node `cron_job_t` and the parsed expression `cron_expr_t` are internal and not exposed.

## How It Works

### Scheduler Thread Loop

The scheduler thread is created during `cron_alloc()` and loops through the following:

1. Read the current wall-clock time via `wallclock_gettime(&tm, tz)`
2. Acquire the lock and iterate the job list, matching each non-removed job against its expression; matched jobs are recorded into a snapshot array (their reference count is incremented so they cannot be freed during execution)
3. Release the lock and invoke each matched job's callback; after each callback, re-acquire the lock, decrement the reference count, and remove one-shot jobs from the list
4. Wait on the semaphore `sem` until the next minute boundary (`timeout = (60 - tm.second) * 1000` ms)
5. Return to step 1

`cron_free()` sets `running` to 0 and posts `sem` to wake the sleeping thread so it exits the loop immediately, instead of waiting up to 60 seconds for the minute-level sleep to elapse.

### Deduplication and One-shot Jobs

Each job records the "minute stamp" of its last firing (packed from year/month/day/hour/minute), so it fires at most once per minute; one-shot jobs are removed from the list and freed after their callback completes.

### Reference Counting

The reference count is incremented when a job is matched and decremented after its callback; both `cron_remove()` and one-shot auto-removal set the `removed` flag first, then decrement, and invoke `destroy(data)` to reclaim `data` and free the job memory only when the count reaches zero. Therefore it is safe to call `cron_remove()` from within a job callback to remove itself or another job.

## API

| Function | Description |
|----------|-------------|
| `cron_alloc(tz)` | Create a scheduler and start the background thread, `tz` is the timezone string, may be `NULL` |
| `cron_free(cron)` | Stop the thread and free the scheduler along with all jobs |
| `cron_add(cron, name, expr, oneshot, exec, destroy, data)` | Add a job, `name` must be unique, non-zero `oneshot` means fire only once; `exec` is the callback fired when due, `destroy` reclaims `data` when the job is destroyed, may be `NULL` |
| `cron_remove(cron, name)` | Remove a job by name |
| `cron_foreach(cron, cb, data)` | Iterate over non-removed jobs, calling `cb(name, oneshot, data)` for each |

Return values: `cron_alloc` returns a pointer on success, `NULL` on failure; `cron_add`/`cron_remove` return `1` on success, `0` on failure.

## Expression Format

```
*    *    *    *    *
│    │    │    │    │
│    │    │    │    └── day of week (0 - 6, 0 = Sunday)
│    │    │    └─────── month (1 - 12)
│    │    └──────────── day of month (1 - 31)
│    └───────────────── hour (0 - 23)
└────────────────────── minute (0 - 59)
```

The expression consists of 5 fields separated by spaces:

| Field | Range | Description |
| --- | --- | --- |
| Minute | 0-59 | Minute of the hour |
| Hour | 0-23 | Hour, 0-based |
| Day of month | 1-31 | Day of the month, 1-based |
| Month | 1-12 or JAN-DEC | Month, name abbreviations allowed |
| Day of week | 0-6 or SUN-SAT | Day of week, 0=Sunday (7 equals 0) |

Each field accepts:

| Syntax | Description |
| --- | --- |
| `*` | Any value |
| `n` | A single value |
| `a-b` | Range |
| `a-b/s` | Range with step `s` |
| `*/s` | Any value with step `s` |
| `a/s` | From `a` up to the field maximum with step `s`, e.g. `5/15` means 5,20,35,50 |
| `a,b,c` | List, combining any of the above |

Month and day-of-week accept case-insensitive 3-letter abbreviations: `JAN`..`DEC`, `SUN`..`SAT`.

## Day of Month and Day of Week

### `7` Equals `0`

In the day-of-week field, `7` is an alias for `0` (Sunday). Thus `0-7`, `0,7`, and `1-7` all correctly include Sunday.

### Combination Rule

Follows Vixie cron semantics:

- When both **day of month** and **day of week** are explicitly specified (neither is `*`), a match on either wins (**OR**)
- Otherwise both must match (**AND**)

For example, `30 4 1,15 * 5` runs at 4:30 on the 1st and 15th of each month, or every Friday.

## Usage Examples

### Basic Usage

```c
#include <kernel/core/cron.h>

static void tick(void * data)
{
    LOG("cron tick\n");
}

void demo(void)
{
    struct cron_t * cron = cron_alloc(NULL);                 /* default timezone */
    cron_add(cron, "every-5min", "*/5 * * * *", 0, tick, NULL, NULL);
    /* ... runs and schedules continuously ... */
    cron_free(cron);
}
```

### One-shot Job

```c
/* Fires once at the next 3:30, then is automatically removed from the list */
cron_add(cron, "once", "30 3 * * *", 1, tick, NULL, NULL);
```

### Removing a Job from Within a Callback

```c
static void job(void * data)
{
    struct cron_t * cron = (struct cron_t *)data;
    cron_remove(cron, "other-job");   /* safe: reference-counted */
}

void setup(struct cron_t * cron)
{
    cron_add(cron, "self", "*/1 * * * *", 0, job, NULL, cron);
}
```

### Job with Data Ownership

`data` is typically a heap-allocated context prepared for the callback. On `cron_add` success its ownership transfers to the job, and `destroy` reclaims it when the job is destroyed; on failure it remains owned by the caller.

```c
static void exec_msg(void * data)
{
    LOG("cron: %s\n", (const char *)data);
}

static void free_msg(void * data)
{
    if(data)
        xos_mem_free(data);
}

void demo(struct cron_t * cron)
{
    char * msg = xos_strdup("hello");
    if(!cron_add(cron, "msg", "*/1 * * * *", 0, exec_msg, free_msg, msg))
        xos_mem_free(msg);   /* add failed, caller frees */
}
```

### Iterating Jobs

```c
static void list_cb(char * name, int oneshot, void * data)
{
    LOG("job: %s, oneshot=%d\n", name, oneshot);
}

cron_foreach(cron, list_cb, NULL);
```

### Common Expressions

| Expression | Meaning |
| --- | --- |
| `*/5 * * * *` | Every 5 minutes |
| `0 * * * *` | Every hour on the hour |
| `30 3 * * *` | Daily at 3:30 |
| `0 9 * * 1` | Monday 9:00 |
| `0 0 1 * *` | 1st of every month at 0:00 |
| `0 0 * * 5` | Friday 0:00 |
| `0 0 * * 0` | Sunday 0:00 (`7` is equivalent) |
| `0 0 1 1 *` | January 1st at 0:00 |
| `30 4 1,15 * 5` | 1st and 15th of each month, or Friday, at 4:30 |
| `*/15 * * * *` | Every 15 minutes |

## Notes

- The scheduler depends on the thread system; ensure the platform supports threads (`xstar_feature_thread()`) before use, otherwise `cron_alloc()` returns `NULL`
- Job `name` must be unique; adding a duplicate name fails
- On `cron_add` success, ownership of `data` transfers to the job: `destroy(data)` is called when the job is destroyed (one-shot auto-removal, `cron_remove`, or `cron_free`); when `destroy` is `NULL` cron leaves `data` untouched and the caller manages its lifetime
- On `cron_add` failure, ownership is not transferred and `data` is still freed by the caller
- The time source is the wall clock `wallclock_gettime()`, with `tz` forwarded as-is; `wallclock_time_t.week` is defined as `(days since 1970-01-01 + 4) % 7`, i.e. `0 = Sunday`, matching the cron standard
- Scheduling precision is at the minute level, checked once per minute boundary; if a minute is skipped (e.g. the system is busy), jobs for that minute are not retroactively fired
- `cron_add`/`cron_remove`/`cron_foreach` may be called from any thread, including from within a job callback
- `cron_free` wakes and waits for the scheduler thread to exit before freeing all jobs; the handle must not be used afterward
