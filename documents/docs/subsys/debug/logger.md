# 日志器 (logger)

基于环形缓冲区的日志系统，支持时间戳自动添加，写入路径中断安全、不阻塞，适用于内核与驱动的调试输出。

## 配置

日志系统通过 Kconfig 选项控制：

| 配置项 | 说明 |
|--------|------|
| `CONFIG_XSTAR_LOG` | 启用日志系统，启用后 `LOG` 宏才生效 |
| `CONFIG_XSTAR_LOGGER_SIZE` | 环形缓冲区大小（字节），默认 8192 |
| `CONFIG_CMD_DMESG` | 启用 `dmesg` 命令，用于查看与跟踪日志 |

## 工作原理

日志器采用"生产者-消费者"模型，`logger()` 只负责写入缓冲区，不做任何控制台输出：

- **写入**（`logger_push`）：将字符逐个写入 `head` 位置，`head` 前移；当缓冲区满时，`tail` 跟随前移，覆盖最旧的数据
- **读取**（`logger_dump`）：从 `tail`（或调用者持有的游标位置）非破坏性地读取，通过回调函数输出，读取后缓冲区内容保留
- **清空**（`logger_clear`）：重置 `head` 和 `tail`，清空整个缓冲区
- **线程安全**：所有缓冲区操作通过自旋锁（`spinlock`）保护，裸机平台的自旋锁实现会保存/恢复中断，因此写入路径可在中断上下文中安全调用

每次调用 `logger()` 时，会自动在日志内容前添加微秒级时间戳，格式为 `[秒.微秒]`，然后仅写入环形缓冲区并立即返回，不做任何 I/O 操作。

## dmesg 命令

日志通过 shell 的 `dmesg` 命令查看：

```
dmesg        # 打印缓冲区内的全部日志（非破坏性，可反复执行）
dmesg -c     # 打印后清空缓冲区
dmesg -w     # 先回放已有日志，然后持续打印新日志，Ctrl-C 退出
```

## API

| 函数 | 说明 |
|------|------|
| `LOG(fmt, arg...)` | 日志输出宏，格式同 `printf`；未启用 `CONFIG_XSTAR_LOG` 时为空操作 |
| `logger(fmt, ...)` | 底层日志函数，自动添加时间戳并写入环形缓冲区 |
| `logger_dump(pos, cb, data)` | 非破坏性地读取日志并通过回调 `cb(buf, len, data)` 输出；`pos` 为 `NULL` 时读取整个保留缓冲区，否则从 `*pos` 增量读取并回写游标，游标被覆盖或清空时自动钳位到 `tail` |
| `logger_clear()` | 清空日志缓冲区 |

## 用法示例

```c
#include <kernel/core/logger.h>

/* 基本日志输出 */
LOG("value = %d\n", value);
LOG("device %s initialized, rate = %lu\n", name, rate);
```

在 shell 中查看：

```
xstar: /# dmesg
[    1.000123] value = 42
[    1.000456] device uart-pl011 initialized, rate = 115200
```

## 说明

- `LOG` 宏在未启用 `CONFIG_XSTAR_LOG` 时编译为空操作，无任何运行时开销
- 时间戳通过 `ktime_get()` 获取，精度为微秒级
- 缓冲区满时采用覆盖策略，保留最新日志，丢弃最旧日志
- 写入路径只做格式化与内存拷贝，无阻塞 I/O，可在中断上下文中调用
- 日志不会实时输出到控制台，需要通过 `dmesg` 命令查看，`dmesg -w` 可持续跟踪新日志
- 日志始终生效，若需彻底关闭，通过 `CONFIG_XSTAR_LOG` 在编译期裁剪
