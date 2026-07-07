# 性能分析器 (profiler)

基于哈希表的性能分析工具，用于测量代码片段的执行时间和调用次数。

## 结构体

```c
struct profiler_t {
    uint64_t begin;    /* 开始时间戳 (ns) */
    uint64_t end;      /* 结束时间戳 (ns) */
    uint64_t elapsed;  /* 累计耗时 (ns) */
    uint64_t count;    /* 调用次数 */
};
```

## API

| 函数 | 说明 |
|------|------|
| `profiler_begin(p)` | 开始计时，记录当前时间到 `begin` |
| `profiler_end(p)` | 结束计时，累加耗时到 `elapsed`，`count++` |
| `profiler_search(name)` | 查找或创建指定名称的分析器 |
| `profiler_foreach(cb)` | 遍历所有分析器，回调返回名称、平均耗时、调用次数 |
| `profiler_clear()` | 清空所有分析器数据 |

## 用法示例

```c
#include <kernel/core/profiler.h>

struct profiler_t * p = profiler_search("my-function");

profiler_begin(p);
/* ... 待测量的代码 ... */
profiler_end(p);

/* 打印所有分析结果 */
profiler_foreach([](const char * name, uint64_t count, uint64_t time) {
    shell_printf("%s: avg=%lluns, count=%llu\n", name, time, count);
});
```

## 说明

- 分析器名称通过哈希表管理，同名分析器共享数据
- `profiler_search` 在哈希表中查找，不存在时自动创建并初始化
- `profiler_begin/end` 为内联函数，使用 `ktime_get()` 获取纳秒级时间戳
- `profiler_foreach` 按名称排序输出，回调中 `time` 为平均耗时（总耗时/次数）
- `profiler_clear` 清空哈希表，重置所有数据
