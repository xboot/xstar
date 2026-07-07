# 持久化 (setting)

键值对持久化存储模块，提供系统配置的读写、延迟写入和自动持久化功能，支持块设备和文件两种存储后端，通过 CRC32 校验保证数据完整性。

## 数据结构

```c
struct setting_t {
    struct timer_t timer;       /* 延迟写入定时器 */
    struct hmap_t * map;        /* 键值对哈希表 */
    struct mutex_t lock;        /* 互斥锁，保护并发访问 */
    int dirty;                  /* 脏标志，1 表示有未写入的修改 */

    uint64_t (*capacity)(struct setting_t *);              /* 获取存储容量 */
    uint64_t (*read)(struct setting_t *, void *, uint64_t, uint64_t);   /* 读取数据 */
    uint64_t (*write)(struct setting_t *, void *, uint64_t, uint64_t);  /* 写入数据 */
    void (*sync)(struct setting_t *);                      /* 刷写到介质 */
    void * priv;                /* 后端私有数据 */
};
```

## 工作原理

### 存储后端

初始化时按优先级自动检测存储后端：

1. **块设备后端**：在已注册的块设备中查找名称包含 `"setting"` 的设备，使用 `block_read`/`block_write`/`block_sync` 直接读写
2. **文件后端**：在 XFS 文件系统中创建/打开 `setting.db` 文件，容量固定为 1MB

若两者均不可用，持久化功能不可用，但内存中的键值对仍可正常读写。

### 文件格式

持久化数据采用自定义二进制格式：

```
偏移  大小    内容
0     4      魔数 "STAR"
4     4      CRC32 校验值（覆盖偏移 8 到末尾）
8     4      数据长度（小端序，字节数）
12    N      键值对数据，格式为 key:value|key:value|...
```

键值对按哈希表排序后写入，CRC32 校验覆盖长度字段和数据内容，加载时校验不通过则丢弃数据。

### 延迟写入

修改操作（`setting_set`/`setting_clear`）不会立即写入存储，而是设置脏标志并启动 10 秒定时器。定时器到期后，`setting_timer_function` 将所有非 `volatile.` 前缀的键值对序列化并写入存储。

### Volatile 键

以 `volatile.` 为前缀的键不会被持久化到存储介质，仅在内存中生效。适用于临时配置和运行时状态。

### 强制同步

`setting_sync()` 将定时器提前到 1ms 后触发，并阻塞等待直到脏标志清除，确保数据已写入存储。

## API

| 函数 | 说明 |
|------|------|
| `setting_set(key, value)` | 设置键值对，`value` 为 NULL 时删除该键；非 `volatile.` 键触发延迟写入 |
| `setting_get(key, def)` | 获取键值，键不存在时返回 `def` |
| `setting_clear()` | 清空所有键值对，触发延迟写入 |
| `setting_sync()` | 强制将所有修改立即写入存储，阻塞直到完成 |
| `setting_foreach(cb)` | 遍历所有键值对，按键名排序，对每对调用回调函数 |
| `do_init_setting()` | 初始化持久化模块，检测后端并加载已有数据（系统启动时自动调用） |

## 用法示例

### 读写配置

```c
#include <kernel/core/setting.h>

/* 写入配置 */
setting_set("wifi.ssid", "MyNetwork");
setting_set("wifi.password", "12345678");

/* 读取配置 */
const char * ssid = setting_get("wifi.ssid", "default");
const char * pwd = setting_get("wifi.password", NULL);

/* 删除配置 */
setting_set("wifi.ssid", NULL);
```

### Volatile 键

```c
/* volatile. 前缀的键不持久化，重启后丢失 */
setting_set("volatile.session_id", "abc123");
const char * sid = setting_get("volatile.session_id", NULL);
```

### 遍历所有配置

```c
static void print_setting(const char * key, const char * value)
{
    shell_printf("%s = %s\r\n", key, value);
}

void list_settings(void)
{
    setting_foreach(print_setting);
}
```

### 强制同步

```c
/* 修改关键配置后立即持久化 */
setting_set("system.boot_count", "42");
setting_sync();  /* 阻塞直到写入完成 */
```

## 说明

- 所有操作通过互斥锁保护，线程安全
- 延迟写入间隔为 10 秒，避免频繁写入损耗存储介质
- `volatile.` 前缀的键不参与持久化，序列化时遇到第一个 `volatile.` 键即停止
- 文件格式使用 CRC32 校验，加载时校验失败则忽略全部数据
- `do_init_setting()` 在 `xstar_init()` 中自动调用，无需手动调用
- `setting_sync()` 内部使用 `xos_thread_sleep()` 轮询等待，不能在中断上下文调用
