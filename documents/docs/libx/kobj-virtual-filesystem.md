# KOBJ虚拟文件系统 (kobj)

本文档详细介绍 XSTAR 的 KOBJ（Kernel Object）虚拟文件系统，包括数据结构、API 接口和使用示例。

## 目录

- [引言](#引言)
- [KOBJ 节点类型](#kobj-节点类型)
- [KOBJ 结构体](#kobj-结构体)
- [KOBJ API](#kobj-api)
- [使用示例](#使用示例)
- [实际应用](#实际应用)
- [API 参考](#api-参考)

## 引言

### KOBJ 的作用

KOBJ（Kernel Object）即内核对象，其层次结构表现为一棵倒置的树，用来管理各种设备、驱动、运行时状态等。

在文件系统中，通过 sysfs 接口绑定 kobj 节点，来实现对该节点的读写操作。sysfs 接口有两种基本用途：

1. **组件外部操作接口**：应用可以通过此接口操作设备
2. **调试驱动模块或查看运行时状态**：通过读写 kobj 节点获取设备信息

### 核心价值

实现 kobj 可以对任何形态的组件进行高度抽象，是各种组件的基石。系统提供了一组 API，以方便管理 kobj。

理解了 kobj，对理解 XSTAR 中的各种核心组件（如设备驱动、设备管理等）有相当大的帮助，建议细心阅读。

## KOBJ 节点类型

kobj 有两种基本类型：

### 目录类型（KOBJ_TYPE_DIR）

- 可以挂接子节点
- 类似文件系统中的目录
- 不提供读写接口
- 用于组织节点层次结构

### 文件类型（KOBJ_TYPE_REG）

- 代表树的末端节点
- 不可拥有子节点
- 提供读或写接口
- 用于存储和传递数据

**枚举定义：**
```c
enum kobj_type_t {
    KOBJ_TYPE_DIR,
    KOBJ_TYPE_REG,
};
```

## KOBJ 结构体

### kobj_t 结构定义

kobj 结构体拥有名字、节点类型、父节点指针、当前节点链表、子节点链表头、读写接口函数及一个私有数据指针。

```c
struct kobj_t {
    char * name;                           /* kobj 名称 */
    enum kobj_type_t type;                  /* kobj 类型 DIR 或 REG */
    struct kobj_t * parent;                /* kobj 的父节点 */
    struct list_head_t entry;               /* kobj 的链表项 */
    struct list_head_t children;            /* kobj 的子节点链表 */
    ssize_t (*read)(struct kobj_t * kobj, void * buf, size_t size);   /* 读接口 */
    ssize_t (*write)(struct kobj_t * kobj, void * buf, size_t size);  /* 写接口 */
    void * priv;                           /* 私有数据 */
};
```

### 字段说明

| 字段 | 类型 | 说明 |
|------|------|------|
| `name` | `char *` | 节点名称，必须唯一（同一父节点下） |
| `type` | `enum kobj_type_t` | 节点类型：目录或文件 |
| `parent` | `struct kobj_t *` | 父节点指针 |
| `entry` | `struct list_head_t` | 链表项，用于挂载到父节点的 children 链表 |
| `children` | `struct list_head_t` | 子节点链表头 |
| `read` | 函数指针 | 读操作回调函数（仅文件类型有效） |
| `write` | 函数指针 | 写操作回调函数（仅文件类型有效） |
| `priv` | `void *` | 私有数据，可存储任意用户数据 |

### 回调函数类型

```c
typedef ssize_t (*kobj_read_t)(struct kobj_t * kobj, void * buf, size_t size);
typedef ssize_t (*kobj_write_t)(struct kobj_t * kobj, void * buf, size_t size);
```

**回调函数参数：**
- `kobj`：kobj 节点指针
- `buf`：缓冲区指针
- `size`：缓冲区大小

**回调函数返回值：**
- 成功：返回实际读写的字节数
- 失败：返回负值

## KOBJ API

### 获取根节点

获取 kobj 根节点，如果不存在则自动创建。

**函数原型：**
```c
struct kobj_t * kobj_get_root(void);
```

**返回值：**
- 成功：返回根节点指针
- 失败：返回 NULL

**说明：**
- 根节点名称为 "kobj"
- 根节点为全局静态变量
- 根节点也是 sysfs 的顶层目录节点
- 在 mount 文件系统时会挂载到 `/sys` 目录

### 搜索节点

从父节点中搜索名为 name 的子节点。

**函数原型：**
```c
struct kobj_t * kobj_search(struct kobj_t * parent, const char * name);
```

**参数：**
- `parent`：父节点指针
- `name`：要搜索的节点名称

**返回值：**
- 找到：返回节点指针
- 未找到：返回 NULL

**使用示例：**
```c
struct kobj_t *root = kobj_get_root();
struct kobj_t *device = kobj_search(root, "device");
```

### 搜索或创建目录

从父节点中搜索名为 name 的子节点，如果不存在则创建一个目录节点。

**函数原型：**
```c
struct kobj_t * kobj_search_directory_with_create(struct kobj_t * parent, const char * name);
```

**参数：**
- `parent`：父节点指针
- `name`：目录名称

**返回值：**
- 成功：返回目录节点指针
- 失败：返回 NULL

**说明：**
- 如果节点不存在，自动创建一个目录节点
- 如果节点存在但不是目录类型，返回 NULL
- 常用于确保路径上的目录节点存在

**使用示例：**
```c
struct kobj_t *root = kobj_get_root();
struct kobj_t *device = kobj_search_directory_with_create(root, "device");
struct kobj_t *gpio = kobj_search_directory_with_create(device, "gpio");
```

### 分配目录节点

分配一个目录类型的 kobj 节点。

**函数原型：**
```c
struct kobj_t * kobj_alloc_directory(const char * name);
```

**参数：**
- `name`：目录名称

**返回值：**
- 成功：返回分配的目录节点指针
- 失败：返回 NULL

**说明：**
- 仅分配内存并初始化，不添加到父节点
- 需要调用 `kobj_add` 添加到父节点
- 目录节点不提供读写接口

**使用示例：**
```c
struct kobj_t *dir = kobj_alloc_directory("my-directory");
if(dir) {
    kobj_add(parent, dir);
}
```

### 分配文件节点

分配一个文件类型的 kobj 节点。

**函数原型：**
```c
struct kobj_t * kobj_alloc_regular(const char * name, kobj_read_t read, kobj_write_t write, void * priv);
```

**参数：**
- `name`：文件名称
- `read`：读回调函数（可以为 NULL）
- `write`：写回调函数（可以为 NULL）
- `priv`：私有数据指针（可以为 NULL）

**返回值：**
- 成功：返回分配的文件节点指针
- 失败：返回 NULL

**说明：**
- 仅分配内存并初始化，不添加到父节点
- 需要调用 `kobj_add` 添加到父节点
- 文件节点必须提供至少一个回调函数

**使用示例：**
```c
static ssize_t my_read(struct kobj_t * kobj, void * buf, size_t size)
{
    return xos_snprintf(buf, size, "Hello, KOBJ!\n");
}

struct kobj_t *file = kobj_alloc_regular("my-file", my_read, NULL, NULL);
if(file) {
    kobj_add(parent, file);
}
```

### 添加节点

将一个节点（目录或文件）添加到父目录节点。

**函数原型：**
```c
int kobj_add(struct kobj_t * parent, struct kobj_t * kobj);
```

**参数：**
- `parent`：父节点指针（必须是目录类型）
- `kobj`：要添加的节点指针

**返回值：**
- 成功：返回 TRUE
- 失败：返回 FALSE

**说明：**
- 父节点必须是目录类型
- 同一父节点下不能有重名子节点
- 自动设置父节点指针

### 删除节点

从父目录节点中删除一个子节点。

**函数原型：**
```c
int kobj_remove(struct kobj_t * parent, struct kobj_t * kobj);
```

**参数：**
- `parent`：父节点指针（必须是目录类型）
- `kobj`：要删除的节点指针

**返回值：**
- 成功：返回 TRUE
- 失败：返回 FALSE

**说明：**
- 仅从父节点中移除，不释放内存
- 需要手动调用 `kobj_free` 释放内存

**使用示例：**
```c
if(kobj_remove(parent, kobj)) {
    kobj_free(kobj);
}
```

### 添加目录节点

快速创建并添加一个目录节点到父节点。

**函数原型：**
```c
int kobj_add_directory(struct kobj_t * parent, const char * name);
```

**参数：**
- `parent`：父节点指针（必须是目录类型）
- `name`：目录名称

**返回值：**
- 成功：返回 TRUE
- 失败：返回 FALSE

**说明：**
- 自动分配目录节点并添加到父节点
- 同一父节点下不能有重名目录
- 如果添加失败，会自动释放分配的内存

**使用示例：**
```c
struct kobj_t *root = kobj_get_root();
kobj_add_directory(root, "device");
kobj_add_directory(root, "class");
```

### 添加文件节点

快速创建并添加一个文件节点到父节点。

**函数原型：**
```c
int kobj_add_regular(struct kobj_t * parent, const char * name, kobj_read_t read, kobj_write_t write, void * priv);
```

**参数：**
- `parent`：父节点指针（必须是目录类型）
- `name`：文件名称
- `read`：读回调函数（可以为 NULL）
- `write`：写回调函数（可以为 NULL）
- `priv`：私有数据指针（可以为 NULL）

**返回值：**
- 成功：返回 TRUE
- 失败：返回 FALSE

**说明：**
- 自动分配文件节点并添加到父节点
- 同一父节点下不能有重名文件
- 如果添加失败，会自动释放分配的内存

**使用示例：**
```c
struct kobj_t *root = kobj_get_root();
kobj_add_regular(root, "version", my_read, NULL, NULL);
```

### 释放节点

释放 kobj 节点占用的内存。

**函数原型：**
```c
int kobj_free(struct kobj_t * kobj);
```

**参数：**
- `kobj`：要释放的节点指针

**返回值：**
- 成功：返回 TRUE
- 失败：返回 FALSE

**说明：**
- 释放节点名称和节点结构体
- 不从父节点中移除节点
- 不递归释放子节点
- 如果目录节点仍有子节点，返回 FALSE（拒绝释放）

**使用示例：**
```c
if(kobj_remove(parent, kobj)) {
    kobj_free(kobj);
}
```

### 递归删除节点

递归删除节点及其所有子节点。

**函数原型：**
```c
int kobj_remove_self(struct kobj_t * kobj);
```

**参数：**
- `kobj`：要删除的节点指针

**返回值：**
- 成功：返回 TRUE
- 失败：返回 FALSE

**说明：**
- 如果节点是目录类型，先递归删除所有子节点
- 从父节点中移除节点
- 释放节点内存
- 常用于清理整个子树

**使用示例：**
```c
struct kobj_t *my_dir = kobj_search(root, "my-directory");
if(my_dir) {
    kobj_remove_self(my_dir);
}
```

## 使用示例

### 示例 1：创建简单的 kobj 树

```c
#include <xstar.h>
#include <libx/kobj.h>

static void create_simple_kobj_tree(void)
{
    struct kobj_t *root, *device, *gpio, *file;

    /* 获取根节点 */
    root = kobj_get_root();

    /* 创建 device 目录 */
    device = kobj_search_directory_with_create(root, "device");

    /* 创建 device/gpio 子目录 */
    gpio = kobj_search_directory_with_create(device, "gpio");

    /* 创建文件节点 */
    file = kobj_alloc_regular("test-file", NULL, NULL, NULL);
    if(file) {
        kobj_add(gpio, file);
    }
}
```

### 示例 2：提供读写接口

```c
static int counter = 0;

static ssize_t counter_read(struct kobj_t * kobj, void * buf, size_t size)
{
    return xos_snprintf(buf, size, "%d\n", counter);
}

static ssize_t counter_write(struct kobj_t * kobj, void * buf, size_t size)
{
    char str[32];
    int len = (size < sizeof(str) - 1) ? size : sizeof(str) - 1;

    xos_memcpy(str, buf, len);
    str[len] = '\0';
    counter = xos_atol(str);

    return size;
}

static void create_counter_kobj(void)
{
    struct kobj_t *root, *test;

    root = kobj_get_root();
    test = kobj_search_directory_with_create(root, "test");

    kobj_add_regular(test, "counter", counter_read, counter_write, &counter);
}
```

### 示例 3：使用私有数据

```c
struct my_data_t {
    int value1;
    int value2;
};

static ssize_t my_data_read(struct kobj_t * kobj, void * buf, size_t size)
{
    struct my_data_t *data = (struct my_data_t *)kobj->priv;
    return xos_snprintf(buf, size, "value1=%d,value2=%d\n", data->value1, data->value2);
}

static void create_kobj_with_priv(void)
{
    struct my_data_t *data;
    struct kobj_t *root, *test;

    data = xos_mem_malloc(sizeof(struct my_data_t));
    if(!data)
        return;

    data->value1 = 10;
    data->value2 = 20;

    root = kobj_get_root();
    test = kobj_search_directory_with_create(root, "test");

    kobj_add_regular(test, "my-data", my_data_read, NULL, data);
}
```

### 示例 4：完整的设备 kobj 创建

```c
static ssize_t device_info_read(struct kobj_t * kobj, void * buf, size_t size)
{
    struct device_t *dev = (struct device_t *)kobj->priv;

    return xos_snprintf(buf, size, "name=%s\ndriver=%s\n",
                      dev->name, dev->driver->name);
}

static void create_device_kobj(struct device_t * dev)
{
    struct kobj_t *root, *device_dir, *dev_dir;

    /* 获取根节点 */
    root = kobj_get_root();

    /* 创建 device 目录 */
    device_dir = kobj_search_directory_with_create(root, "device");

    /* 创建设备专属目录 */
    dev_dir = kobj_search_directory_with_create(device_dir, dev->name);

    /* 添加设备信息文件 */
    kobj_add_regular(dev_dir, "info", device_info_read, NULL, dev);

    /* 添加其他属性文件... */
}
```

## 实际应用

### 在设备驱动中使用

每个驱动模块都包含一个 kobj 对象，在注册设备时，会依据设备的名称及类型自动创建一组 kobj 对象。

**设备节点路径示例：**
```
/kobj/device/framebuffer/fb.0/
├── info
├── width
├── height
└── format
```

**实现示例：**
```c
static struct device_t * my_device_probe(struct driver_t * drv, struct dtnode_t * n)
{
    struct device_t *dev;
    struct kobj_t *root, *device_dir, *dev_dir;

    /* 创建设备 */
    dev = register_my_device(drv, pdat);
    if(!dev)
        return NULL;

    /* 创建 kobj 节点 */
    root = kobj_get_root();
    device_dir = kobj_search_directory_with_create(root, "device");
    dev_dir = kobj_search_directory_with_create(device_dir, dev->name);

    kobj_add_regular(dev_dir, "info", my_device_info_read, NULL, dev);

    return dev;
}
```

### 系统级 kobj 树结构

```
/kobj/
├── device/              /* 设备节点 */
│   ├── framebuffer/
│   │   └── fb.0/
│   │       ├── info
│   │       ├── width
│   │       └── height
│   ├── gpio/
│   │   └── gpio.0/
│   │       └── info
│   └── ...
├── class/               /* 类节点 */
│   ├── memory/
│   │   └── meminfo
│   └── ...
└── driver/             /* 驱动节点 */
    ├── clk/
    │   └── clk.0/
    │       └── rate
    └── ...
```

### 通过文件系统访问

kobj 节点通过 sysfs 文件系统绑定，可以通过文件系统接口访问：

```bash
# 查看设备信息
cat /sys/kobj/device/framebuffer/fb.0/info

# 查看屏幕宽度
cat /sys/kobj/device/framebuffer/fb.0/width

# 设置时钟频率
echo 24000000 > /sys/kobj/driver/clk/clk.0/rate

# 查看内存信息
cat /sys/kobj/class/memory/meminfo
```

## API 参考

### 节点管理

```c
struct kobj_t * kobj_get_root(void);
struct kobj_t * kobj_search(struct kobj_t * parent, const char * name);
struct kobj_t * kobj_search_directory_with_create(struct kobj_t * parent, const char * name);
int kobj_free(struct kobj_t * kobj);
int kobj_add(struct kobj_t * parent, struct kobj_t * kobj);
int kobj_remove(struct kobj_t * parent, struct kobj_t * kobj);
int kobj_remove_self(struct kobj_t * kobj);
```

### 节点创建

```c
struct kobj_t * kobj_alloc_directory(const char * name);
struct kobj_t * kobj_alloc_regular(const char * name, kobj_read_t read, kobj_write_t write, void * priv);
```

### 快捷添加

```c
int kobj_add_directory(struct kobj_t * parent, const char * name);
int kobj_add_regular(struct kobj_t * parent, const char * name, kobj_read_t read, kobj_write_t write, void * priv);
```

### 回调函数类型

```c
typedef ssize_t (*kobj_read_t)(struct kobj_t * kobj, void * buf, size_t size);
typedef ssize_t (*kobj_write_t)(struct kobj_t * kobj, void * buf, size_t size);
```

## 最佳实践

### 1. 节点命名

- 使用小写字母和连字符（kebab-case）
- 名称要具有描述性
- 避免使用特殊字符
- 同一父节点下名称必须唯一

### 2. 错误处理

- 始终检查函数返回值
- 添加失败时释放已分配的内存
- 使用 `kobj_remove_self` 清理整个子树

### 3. 线程安全

- kobj 不使用锁，所有操作均不加锁
- kobj 树通常在启动阶段（initcall）单线程构建，运行时以读访问为主
- 回调函数中可以安全调用 kobj_search 等读接口
- 回调函数中不要调用 kobj_add/remove（可能导致遍历时链表损坏）

### 4. 内存管理

- 添加节点前先分配，失败后立即释放
- 使用私有数据时，确保生命周期与节点一致
- 递归删除时注意释放私有数据

### 5. 回调函数设计

- 读函数：返回格式化的字符串数据
- 写函数：解析输入并更新状态
- 使用私有数据传递上下文
- 返回实际读写的字节数

## 总结

kobj 是 XSTAR 中用于管理内核对象的虚拟文件系统，具有以下特点：

1. **树形结构**：层次化的组织方式，便于管理
2. **类型区分**：目录和文件两种类型，用途明确
3. **灵活扩展**：通过读写回调实现自定义功能
4. **无锁设计**：kobj 树在启动阶段构建，运行时以读访问为主，无需加锁
5. **易于访问**：通过文件系统接口操作

kobj 在设备驱动、系统监控、调试等方面有广泛应用，是理解 XSTAR 核心组件的关键。

对于如何使用 kobj，请参考驱动模块里的实现。如何通过文件系统访问 kobj，请参考虚拟文件系统中的 sysfs 文件系统的实现。
