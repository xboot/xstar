# Profiler (profiler)

A hash table based profiling tool for measuring execution time and call count of code sections.

## Struct

```c
struct profiler_t {
    uint64_t begin;    /* Start timestamp (ns) */
    uint64_t end;      /* End timestamp (ns) */
    uint64_t elapsed;  /* Accumulated time (ns) */
    uint64_t count;    /* Call count */
};
```

## API

| Function | Description |
|----------|-------------|
| `profiler_begin(p)` | Start timing, record current time into `begin` |
| `profiler_end(p)` | Stop timing, accumulate elapsed time into `elapsed`, `count++` |
| `profiler_search(name)` | Find or create a profiler with the given name |
| `profiler_foreach(cb)` | Iterate over all profilers, callback returns name, average time, call count |
| `profiler_clear()` | Clear all profiler data |

## Usage Example

```c
#include <kernel/core/profiler.h>

struct profiler_t * p = profiler_search("my-function");

profiler_begin(p);
/* ... code to be measured ... */
profiler_end(p);

/* Print all profiling results */
profiler_foreach([](const char * name, uint64_t count, uint64_t time) {
    shell_printf("%s: avg=%lluns, count=%llu\n", name, time, count);
});
```

## Notes

- Profiler names are managed via a hash table; profilers with the same name share data
- `profiler_search` looks up the hash table, auto-creates and initializes if not found
- `profiler_begin/end` are inline functions that use `ktime_get()` to obtain nanosecond timestamps
- `profiler_foreach` outputs sorted by name, `time` in the callback is the average time (total / count)
- `profiler_clear` clears the hash table, resetting all data
