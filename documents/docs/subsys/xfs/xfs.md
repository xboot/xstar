# 文件系统 (xfs)

XSTAR 虚拟文件系统，通过可插拔的归档器（archiver）后端统一访问不同存储介质，支持 CPIO 归档、真实目录和 KOBJ 虚拟文件系统，提供挂载、文件读写和目录遍历等标准文件操作。

## 数据结构

### 文件系统上下文

```c
struct xfs_context_t {
    struct xfs_path_t mounts;   /* 挂载点链表头 */
    struct mutex_t lock;        /* 互斥锁，保护挂载点操作 */
};
```

### 挂载点

```c
struct xfs_path_t {
    char * mpath;                       /* 挂载路径 */
    void * mhandle;                     /* 归档器挂载句柄 */
    int writable;                       /* 是否可写 */
    struct xfs_archiver_t * archiver;   /* 归档器实例 */
    struct list_head_t list;            /* 链表节点 */
};
```

### 文件句柄

```c
struct xfs_file_t {
    struct xfs_context_t * ctx;   /* 所属上下文 */
    struct xfs_path_t * path;     /* 所属挂载点 */
    void * fhandle;               /* 文件句柄 */
};
```

## 工作原理

### 归档器架构

XFS 通过归档器接口（`struct xfs_archiver_t`）抽象不同存储后端，每个归档器实现一组统一的操作函数（mount/umount/walk/open/read/write 等）。归档器通过 `register_archiver()` 注册到全局链表，`mount_archiver()` 按注册顺序依次尝试挂载，第一个成功的归档器接管该路径。

### 存储后端

| 归档器 | 说明 | 可写 |
|--------|------|------|
| cpio | CPIO newc 格式归档，从块设备读取，用于 romdisk | 否 |
| dir | 宿主机真实文件系统目录，通过 XOS 文件接口访问 | 是 |
| sys | KOBJ 虚拟文件系统，挂载路径为 `"sys"`，访问系统内核对象 | 是 |

### 自动挂载

`xfs_alloc()` 创建上下文时自动挂载三个路径：

1. `"blk-romdisk.0"` — 只读，由 cpio 归档器接管，访问编译时嵌入的 romdisk
2. `"sys"` — 可读写，由 sys 归档器接管，访问 KOBJ 虚拟文件系统
3. 当前工作目录 — 可读写，由 dir 归档器接管，访问宿主机文件系统

### 查找顺序

文件查找采用逆序遍历挂载点链表（后挂载的优先）：

- **读操作**（open_read/isdir/isfile/walk）：从最后挂载的点向前查找，第一个匹配的返回
- **写操作**（open_write/open_append/mkdir/remove）：从最后挂载的可写点向前查找

### 路径规范化

所有路径在操作前经过 `normal_path()` 处理：

- 去除前导 `/`
- 拒绝 `.` 和 `..` 路径分量（安全限制，防止路径穿越）
- 拒绝 `:` 和 `\\` 字符

## API

### 上下文管理

| 函数 | 说明 |
|------|------|
| `xfs_alloc()` | 创建文件系统上下文，自动挂载 romdisk、sys 和当前工作目录 |
| `xfs_free(ctx)` | 释放上下文，卸载所有挂载点 |

### 挂载管理

| 函数 | 说明 |
|------|------|
| `xfs_mount(ctx, path, writable)` | 挂载路径，`writable` 为是否请求写权限 |
| `xfs_umount(ctx, path)` | 卸载路径 |

### 目录操作

| 函数 | 说明 |
|------|------|
| `xfs_walk(ctx, name, cb, data)` | 遍历目录，对每个条目调用回调 `cb(dir, name, data)` |
| `xfs_isdir(ctx, name)` | 判断是否为目录 |
| `xfs_isfile(ctx, name)` | 判断是否为文件 |
| `xfs_mode(ctx, name)` | 获取文件模式 |
| `xfs_mkdir(ctx, name)` | 创建目录（仅可写挂载点） |
| `xfs_remove(ctx, name)` | 删除文件或目录（仅可写挂载点） |

### 文件操作

| 函数 | 说明 |
|------|------|
| `xfs_open_read(ctx, name)` | 以只读方式打开文件 |
| `xfs_open_write(ctx, name)` | 以只写方式打开文件（仅可写挂载点） |
| `xfs_open_append(ctx, name)` | 以追加方式打开文件（仅可写挂载点） |
| `xfs_read(file, buf, size)` | 读取文件内容，返回实际读取字节数 |
| `xfs_write(file, buf, size)` | 写入文件内容，返回实际写入字节数 |
| `xfs_seek(file, offset)` | 设置文件读写位置 |
| `xfs_tell(file)` | 获取当前读写位置 |
| `xfs_length(file)` | 获取文件长度 |
| `xfs_flush(file)` | 刷新文件缓冲区 |
| `xfs_close(file)` | 关闭文件并释放资源 |

## 用法示例

### 读取文件

```c
#include <kernel/xfs/xfs.h>

struct xfs_context_t * ctx = xfs_alloc();
struct xfs_file_t * file = xfs_open_read(ctx, "dtree/default.json");
if(file)
{
    int64_t len = xfs_length(file);
    char * buf = xos_mem_malloc(len + 1);
    if(buf)
    {
        xfs_read(file, buf, len);
        buf[len] = '\0';
        LOG("content: %s\n", buf);
        xos_mem_free(buf);
    }
    xfs_close(file);
}
xfs_free(ctx);
```

### 写入文件

```c
struct xfs_context_t * ctx = xfs_alloc();
struct xfs_file_t * file = xfs_open_write(ctx, "setting.db");
if(file)
{
    const char * data = "hello world";
    xfs_write(file, (void *)data, strlen(data));
    xfs_flush(file);
    xfs_close(file);
}
xfs_free(ctx);
```

### 遍历目录

```c
static void list_cb(const char * dir, const char * name, void * data)
{
    shell_printf("%s/%s\r\n", dir, name);
}

void list_files(void)
{
    struct xfs_context_t * ctx = xfs_alloc();
    xfs_walk(ctx, "", list_cb, NULL);
    xfs_free(ctx);
}
```

### 访问 KOBJ 虚拟文件系统

```c
struct xfs_context_t * ctx = xfs_alloc();
struct xfs_file_t * file = xfs_open_read(ctx, "sys/clk/clk-fixed.0/summary");
if(file)
{
    char buf[256];
    int64_t len = xfs_read(file, buf, sizeof(buf) - 1);
    if(len > 0)
    {
        buf[len] = '\0';
        shell_printf("%s\r\n", buf);
    }
    xfs_close(file);
}
xfs_free(ctx);
```

## 说明

- 每个需要文件访问的模块应通过 `xfs_alloc()` 创建独立上下文，使用完毕后调用 `xfs_free()` 释放
- romdisk（`blk-romdisk.0`）为只读，写入操作会自动跳过该挂载点
- 路径分隔符为 `/`，不支持 `.` 和 `..` 路径分量
- 归档器通过 `pure_initcall()` 在系统启动早期初始化
- `mount_archiver()` 按注册顺序尝试挂载，cpio 先于 dir 先于 sys
- KOBJ 虚拟文件系统（sys）提供对系统内核对象（设备、驱动等）的只读/读写访问
