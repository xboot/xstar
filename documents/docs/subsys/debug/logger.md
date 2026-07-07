# 日志器 (logger)

基于环形缓冲区的日志系统，支持时间戳自动添加、运行时启用/禁用，适用于内核与驱动的调试输出。

## 配置

日志系统通过 Kconfig 选项控制：

| 配置项 | 说明 |
|--------|------|
| `CONFIG_XSTAR_LOG` | 启用日志系统，启用后 `LOG` 宏才生效 |
| `CONFIG_XSTAR_LOGGER_SIZE` | 环形缓冲区大小（字节），默认 8192 |

## 工作原理

日志器内部维护一个环形缓冲区（`buffer`），通过 `head` 和 `tail` 两个指针管理读写位置：

- **写入**（`logger_push`）：将字符逐个写入 `head` 位置，`head` 前移；当缓冲区满时，`tail` 跟随前移，覆盖最旧的数据
- **刷新**（`logger_pop`）：从 `tail` 开始逐个读取并通过 `xos_stdio_write` 输出到标准输出，直到追上 `head`
- **线程安全**：所有缓冲区操作通过自旋锁（`spinlock`）保护，支持多线程/多核环境

每次调用 `logger()` 时，会自动在日志内容前添加微秒级时间戳，格式为 `[秒.微秒]`，然后格式化用户输入并立即刷新输出。

## API

| 函数 | 说明 |
|------|------|
| `LOG(fmt, arg...)` | 日志输出宏，格式同 `printf`；未启用 `CONFIG_XSTAR_LOG` 时为空操作 |
| `logger(fmt, ...)` | 底层日志函数，自动添加时间戳并输出 |
| `logger_enable()` | 启用日志输出 |
| `logger_disable()` | 禁用日志输出 |
| `logger_status()` | 查询日志状态，返回 1 表示已启用，0 表示已禁用 |

## 用法示例

```c
#include <kernel/core/logger.h>

/* 基本日志输出 */
LOG("value = %d\n", value);
LOG("device %s initialized, rate = %lu\n", name, rate);

/* 运行时控制 */
logger_disable();  /* 临时关闭日志 */
/* ... 敏感操作 ... */
logger_enable();   /* 恢复日志 */

/* 查询状态 */
if(logger_status())
    LOG("logger is active\n");
```

输出示例：

```
[    1.000123] value = 42
[    1.000456] device uart-pl011 initialized, rate = 115200
```

## 说明

- `LOG` 宏在未启用 `CONFIG_XSTAR_LOG` 时编译为空操作，无任何运行时开销
- 时间戳通过 `ktime_get()` 获取，精度为微秒级
- 缓冲区满时采用覆盖策略，保留最新日志，丢弃最旧日志
- `logger()` 在 `disabled` 状态下直接返回，不执行任何格式化或写入操作
- 日志输出通过 `xos_stdio_write` 实现，具体输出目标取决于平台 XOS 实现
