# XSTAR 开发指南

本文档提供 XSTAR 开发的详细指南，包括代码规范、驱动开发、命令开发、测试等内容。

## 目录

- [代码规范](#代码规范)
- [驱动开发](#驱动开发)
- [命令开发](#命令开发)
- [内核子系统开发](#内核子系统开发)
- [测试开发](#测试开发)
- [XOS 平台移植](#xos-平台移植)
- [常见开发任务](#常见开发任务)
- [调试技巧](#调试技巧)

## 代码规范

### 文件命名

- 源文件：`kebab-case.c`（如 `clk-fixed.c`、`i2c-gpio.c`）
- 头文件：`kebab-case.h`（与源文件同名）

### 文件头注释

每个源文件必须包含 MIT 许可证头：

```c
/*
 * driver/clk/clk-fixed.c
 *
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
```

### 代码格式

- **缩进**：使用 Tab（不使用空格）
- **行宽**：约 120 字符（不严格强制，但保持合理）
- **大括号**：函数和控制结构使用 Allman 风格（左大括号在下一行）；struct/enum 定义和初始化列表使用 K&R 风格（左大括号在同一行）
- **注释**：尽量减少注释，不主动添加代码注释（除非明确要求）；现有代码中的设备树文档注释和结构体字段注释仅供参考

### 命名规范

- **函数**：`snake_case`（如 `clk_fixed_probe`、`register_driver`）
- **变量**：`snake_case`（如 `pdat`、`rate`、`kobj`）
- **类型**：`snake_case_t` 后缀（如 `struct driver_t`、`enum device_type_t`）
- **常量**：`ALL_CAPS_WITH_UNDERSCORES`（如 `DEVICE_TYPE_MAX_COUNT`、`TRUE`、`FALSE`）
- **宏**：`ALL_CAPS`（如 `ARRAY_SIZE`、`container_of`、`offsetof`）

### 头文件保护

格式：`__PATH_TO_FILE_H__`，示例：

```c
#ifndef __XSTAR_DRIVER_CLK_H__
#define __XSTAR_DRIVER_CLK_H__

/* 头文件内容 */

#endif /* __XSTAR_DRIVER_CLK_H__ */
```

### 包含顺序

```c
#include <xstar.h>
#include <xos/xos.h>

#include <driver/clk/clk.h>

#include <libx/json.h>
```

1. 本地项目头文件（`#include <xstar.h>` 或 `#include <xos/xos.h>`）
2. 驱动/内核头文件（如 `#include <driver/clk/clk.h>`）
3. LibX 头文件（如 `#include <libx/json.h>`）
4. 标准/外部头文件（如果有）

### 错误处理

- 布尔函数：成功返回 `TRUE`，失败返回 `FALSE`
- 指针函数：成功返回指针，失败返回 `NULL`
- 始终检查返回值
- 错误路径中清理分配的资源

```c
static struct device_t * my_probe(struct driver_t * drv, struct dtnode_t * n)
{
    struct xxx_pdata_t *pdat;
    struct xxx_t *xxx;

    pdat = xos_mem_malloc(sizeof(struct xxx_pdata_t));
    if(!pdat)
        return NULL;

    xxx = xos_mem_malloc(sizeof(struct xxx_t));
    if(!xxx)
    {
        xos_mem_free(pdat);
        return NULL;
    }

    /* 初始化和注册 */

    return dev;
}
```

### 类型使用

使用标准类型（通过 `<xos/xos.h>` 经 `<xstarcfg.h>` 提供）：

- `uint8_t`、`uint16_t`、`uint32_t`、`uint64_t`
- `int8_t`、`int16_t`、`int32_t`、`int64_t`
- `size_t`、`ssize_t`
- `io_addr_t`
- `NULL`、`TRUE`、`FALSE`（从 `libx/xdef.h`）

## 驱动开发

### 驱动模板

```c
#include <xstar.h>
#include <driver/xxx/xxx.h>

struct xxx_pdata_t {
    /* 私有数据 */
};

static struct device_t * xxx_probe(struct driver_t * drv, struct dtnode_t * n)
{
    struct xxx_pdata_t *pdat;

    /* 解析设备树属性 */
    const char *name = dt_read_string(n, "name", NULL);
    int value = dt_read_int(n, "value", 0);

    /* 分配私有数据 */
    pdat = xos_mem_malloc(sizeof(struct xxx_pdata_t));
    if(!pdat)
        return NULL;

    /* 初始化硬件 */

    /* 创建并注册设备 */
    return register_xxx(drv, pdat);
}

static void xxx_remove(struct device_t * dev)
{
    struct xxx_pdata_t *pdat = (struct xxx_pdata_t *)dev->priv;

    /* 释放资源 */

    xos_mem_free(pdat);
}

static void xxx_suspend(struct device_t * dev)
{
}

static void xxx_resume(struct device_t * dev)
{
}

static struct driver_t xxx_driver = {
    .name    = "xxx-driver",
    .probe   = xxx_probe,
    .remove  = xxx_remove,
    .suspend = xxx_suspend,
    .resume  = xxx_resume,
};

static void xxx_driver_init(void)
{
    register_driver(&xxx_driver);
}

static void xxx_driver_exit(void)
{
    unregister_driver(&xxx_driver);
}

driver_initcall(xxx_driver_init);
driver_exitcall(xxx_driver_exit);
```

### 设备树配置

在 `romdisk/boot/boot.json` 中添加设备配置：

```json
{
    "xxx-driver:0": {
        "name": "my-device",
        "value": 42
    }
}
```

设备树键名格式为 `"driver-name:id@address"`：
- `driver-name`：驱动名称，必须与 `driver_t.name` 匹配
- `id`：设备实例编号（可选）
- `address`：物理地址（可选）

设置 `"status": "disabled"` 可跳过设备探测。

### 设备属性读取

| 函数 | 返回类型 | 说明 |
|------|---------|------|
| `dt_read_string(n, name, def)` | `char *` | 读取字符串 |
| `dt_read_int(n, name, def)` | `int` | 读取整数 |
| `dt_read_long(n, name, def)` | `long long` | 读取长整数 |
| `dt_read_bool(n, name, def)` | `int` | 读取布尔值 |
| `dt_read_double(n, name, def)` | `double` | 读取双精度浮点 |
| `dt_read_object(n, name)` | `struct dtnode_t` | 读取子对象 |

### 设备引用

在 JSON 中通过 `"driver-name:id"` 格式引用其他设备：

```json
{
    "led-gpio:0": {
        "gpio": "gpiochip0:10",
        "active-low": true
    }
}
```

在代码中解析引用并查找设备：

```c
static struct device_t * xxx_probe(struct driver_t * drv, struct dtnode_t * n)
{
    const char *gpio_name = dt_read_string(n, "gpio", NULL);
    struct device_t *gpio_dev = search_device(gpio_name, DEVICE_TYPE_GPIOCHIP);
    if(!gpio_dev)
        return NULL;
    /* 使用 gpio_dev */
}
```

### Kbuild 文件

在驱动目录下创建 `Kbuild` 文件：

```makefile
obj-y += core.o
obj-$(CONFIG_DRV_XXX) += xxx-driver.o
```

### 设备类型

系统定义了 51 种设备类型（`enum device_type_t`），驱动注册设备时需指定对应类型。常用类型：

| 类别 | 类型枚举 |
|------|---------|
| 时钟 | `DEVICE_TYPE_CLK`, `DEVICE_TYPE_CLOCKEVENT`, `DEVICE_TYPE_CLOCKSOURCE` |
| GPIO | `DEVICE_TYPE_GPIOCHIP`, `DEVICE_TYPE_IRQCHIP`, `DEVICE_TYPE_RESETCHIP` |
| 通信 | `DEVICE_TYPE_I2C`, `DEVICE_TYPE_SPI`, `DEVICE_TYPE_UART`, `DEVICE_TYPE_NET` |
| 显示 | `DEVICE_TYPE_FRAMEBUFFER`, `DEVICE_TYPE_G2D`, `DEVICE_TYPE_CONSOLE` |
| 音频 | `DEVICE_TYPE_AUDIOCAPTURE`, `DEVICE_TYPE_AUDIOPLAYBACK` |
| 输入 | `DEVICE_TYPE_INPUT`, `DEVICE_TYPE_CAMERA` |
| 存储 | `DEVICE_TYPE_BLOCK`, `DEVICE_TYPE_NVMEM` |
| 输出 | `DEVICE_TYPE_LED`, `DEVICE_TYPE_PWM`, `DEVICE_TYPE_SERVO` |

完整列表参见 `xstar/driver/device.h`。

## 命令开发

### 命令结构体

```c
struct command_t
{
    struct list_head_t list;
    const char * name;
    const char * desc;
    void (*usage)(void);
    int (*exec)(int argc, char ** argv);
};
```

### 命令模板

```c
#include <xstar.h>

static void mycmd_usage(void)
{
    shell_printf("Usage: mycmd [options]\n");
    shell_printf("  mycmd        - do something\n");
    shell_printf("  mycmd -h     - show help\n");
}

static int mycmd_exec(int argc, char ** argv)
{
    shell_printf("My command executed\n");

    if(argc > 1)
    {
        shell_printf("Argument: %s\n", argv[1]);
    }

    return 0;
}

static struct command_t mycmd = {
    .name  = "mycmd",
    .desc  = "My command description",
    .usage = mycmd_usage,
    .exec  = mycmd_exec,
};

static void mycmd_init(void)
{
    register_command(&mycmd);
}

static void mycmd_exit(void)
{
    unregister_command(&mycmd);
}

command_initcall(mycmd_init);
command_exitcall(mycmd_exit);
```

### 命令使用

```bash
# 在 Shell 中执行命令
mycmd
mycmd arg1 arg2
```

### Kbuild 文件

```makefile
obj-$(CONFIG_CMD_MYCMD) += cmd-mycmd.o
```

命令的 Kconfig 选项前缀为 `CONFIG_CMD_*`，每个命令可单独启用/禁用。核心框架 `command.o` 始终编译（`obj-y`）。

## 内核子系统开发

### 子系统接口定义

```c
/* kernel/mysubsystem/my-subsystem.h */
#ifndef __XSTAR_KERNEL_MYSUBSYSTEM_MY_SUBSYSTEM_H__
#define __XSTAR_KERNEL_MYSUBSYSTEM_MY_SUBSYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

struct my_subsystem_handle_t;

struct my_subsystem_handle_t * my_subsystem_open(void);
void my_subsystem_close(struct my_subsystem_handle_t *handle);
int my_subsystem_do_something(struct my_subsystem_handle_t *handle, int arg);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_MYSUBSYSTEM_MY_SUBSYSTEM_H__ */
```

### 子系统实现

```c
/* kernel/mysubsystem/my-subsystem.c */
#include <xstar.h>
#include <kernel/mysubsystem/my-subsystem.h>

struct my_subsystem_handle_t {
    /* 私有数据 */
};

struct my_subsystem_handle_t * my_subsystem_open(void)
{
    struct my_subsystem_handle_t *handle;

    handle = xos_mem_malloc(sizeof(struct my_subsystem_handle_t));
    if(!handle)
        return NULL;

    /* 初始化 */

    return handle;
}

void my_subsystem_close(struct my_subsystem_handle_t *handle)
{
    if(handle)
    {
        /* 清理 */
        xos_mem_free(handle);
    }
}

int my_subsystem_do_something(struct my_subsystem_handle_t *handle, int arg)
{
    if(!handle)
        return -1;

    /* 实现功能 */

    return 0;
}
```

### 子系统初始化

```c
static void my_subsystem_init(void)
{
    /* 初始化子系统 */
}

subsys_initcall(my_subsystem_init);
```

### Kbuild 文件

内核子系统当前无条件编译，Kbuild 文件使用 `obj-y`：

```makefile
obj-y += my-subsystem.o
```

## 测试开发

### 测试框架

XSTAR 使用 wboxtest 测试框架，位于 `packages/wboxtest-0.0.0/`。

### 测试结构体

```c
struct wboxtest_t
{
    struct kobj_t * kobj;
    struct hlist_node_t node;
    const char * group;
    const char * name;
    void * (*setup)(struct wboxtest_t * wbt);
    void (*clean)(struct wboxtest_t * wbt, void * data);
    void (*run)(struct wboxtest_t * wbt, void * data);
};
```

### 测试模板

```c
#include <wboxtest.h>

struct wbt_xxx_pdata_t {
    /* 测试私有数据 */
};

static void * xxx_setup(struct wboxtest_t * wbt)
{
    struct wbt_xxx_pdata_t *pdat;

    pdat = xos_mem_malloc(sizeof(struct wbt_xxx_pdata_t));
    if(!pdat)
        return NULL;

    /* 初始化测试环境 */

    return pdat;
}

static void xxx_clean(struct wboxtest_t * wbt, void * data)
{
    struct wbt_xxx_pdata_t *pdat = (struct wbt_xxx_pdata_t *)data;

    if(pdat)
    {
        /* 清理测试环境 */
        xos_mem_free(pdat);
    }
}

static void xxx_run(struct wboxtest_t * wbt, void * data)
{
    struct wbt_xxx_pdata_t *pdat = (struct wbt_xxx_pdata_t *)data;

    if(!pdat)
        return;

    /* 测试逻辑 */
    assert_true(condition);
    assert_equal(expected, actual);
}

static struct wboxtest_t wbt_xxx = {
    .group = "mygroup",
    .name  = "xxx",
    .setup = xxx_setup,
    .clean = xxx_clean,
    .run   = xxx_run,
};

static void xxx_wbt_init(void)
{
    register_wboxtest(&wbt_xxx);
}

static void xxx_wbt_exit(void)
{
    unregister_wboxtest(&wbt_xxx);
}

wboxtest_initcall(xxx_wbt_init);
wboxtest_exitcall(xxx_wbt_exit);
```

### 断言宏

| 宏 | 说明 |
|-----|------|
| `assert_null(x)` | 断言为 NULL |
| `assert_not_null(x)` | 断言不为 NULL |
| `assert_true(x)` | 断言为真 |
| `assert_false(x)` | 断言为假 |
| `assert_equal(a, b)` | 断言相等 |
| `assert_not_equal(a, b)` | 断言不相等 |
| `assert_string_equal(a, b)` | 断言字符串相等 |
| `assert_string_not_equal(a, b)` | 断言字符串不相等 |
| `assert_memory_equal(a, b, l)` | 断言内存相等 |
| `assert_memory_not_equal(a, b, l)` | 断言内存不相等 |
| `assert_inrange(v, min, max)` | 断言在范围内 |
| `assert_not_inrange(v, min, max)` | 断言不在范围内 |

### 测试运行

```bash
# 运行所有测试
wboxtest

# 列出所有测试
wboxtest -l

# 运行指定组的所有测试
wboxtest thread

# 运行指定测试
wboxtest thread mutex

# 运行指定测试 N 次
wboxtest thread mutex -c 100
```

### Kbuild 文件

```makefile
obj-$(CONFIG_WBOXTEST_MYGROUP) += xxx.o
```

核心框架由 `CONFIG_PKG_WBOXTEST` 控制，子测试组由 `CONFIG_WBOXTEST_*` 控制。

## XOS 平台移植

移植 XOS 到新平台需要实现 `xos_environ_t` 函数指针表，并在项目 `main.c` 中传递给 `xstar_init()`。

### 移植步骤

1. 在项目目录下创建平台实现文件（如 `linux/linux.c` 或 `baremetal/baremetal.c`）
2. 实现 `xos_environ_t` 中所需的函数指针
3. 在 `main.c` 中构建 `xos_environ_t` 实例并调用 `xstar_init(&env, NULL)`
4. 如需协程支持，编写架构特定的协程汇编代码

### xos_environ_t 接口

```c
static struct xos_environ_t env = {
    .mem = {
        .malloc    = my_malloc,
        .free      = my_free,
        .realloc   = my_realloc,
    },
    .dma = {
        .alloc_coherent     = my_dma_alloc_coherent,
        .free_coherent      = my_dma_free_coherent,
        .alloc_noncoherent  = my_dma_alloc_noncoherent,
        .free_noncoherent   = my_dma_free_noncoherent,
        .sync               = my_dma_sync,
    },
    .io = {
        .read8   = my_read8,
        .read16  = my_read16,
        .read32  = my_read32,
        .read64  = my_read64,
        .write8  = my_write8,
        .write16 = my_write16,
        .write32 = my_write32,
        .write64 = my_write64,
    },
    .stdio = {
        .read  = my_stdio_read,
        .write = my_stdio_write,
    },
    .pm = {
        .shutdown = my_shutdown,
        .reboot   = my_reboot,
        .standby  = my_standby,
    },
    .copyright = {
        .uniqueid = my_uniqueid,
        .verify   = my_verify,
    },
    .file = {
        .cwd     = my_cwd,
        .open    = my_file_open,
        .close   = my_file_close,
        .read    = my_file_read,
        .write   = my_file_write,
        .seek    = my_file_seek,
        .tell    = my_file_tell,
        .length  = my_file_length,
        .sync    = my_file_sync,
        .mkdir   = my_mkdir,
        .remove  = my_remove,
        .access  = my_access,
        .isdir   = my_isdir,
        .isfile  = my_isfile,
        .mode    = my_mode,
        .walk    = my_walk,
    },
    .coroutine = {
        .make = my_coroutine_make,
        .jump = my_coroutine_jump,
    },
    .thread = {
        .create  = my_thread_create,
        .destroy = my_thread_destroy,
        .wait    = my_thread_wait,
        .sleep   = my_thread_sleep,
    },
    .mutex = {
        .init   = my_mutex_init,
        .exit   = my_mutex_exit,
        .lock   = my_mutex_lock,
        .trylock = my_mutex_trylock,
        .unlock = my_mutex_unlock,
    },
    .semaphore = {
        .init = my_semaphore_init,
        .exit = my_semaphore_exit,
        .wait = my_semaphore_wait,
        .post = my_semaphore_post,
    },
    .other = {
        /* 其他平台特定操作 */
    },
};

int main(int argc, char * argv[])
{
    platform_init();
    xstar_init(&env, NULL);
    shell_system("shell;");
    xstar_exit();
    platform_exit();
    return 0;
}
```

`xos_environ_init(env)` 会将非 NULL 的函数指针安装到全局 `__xos_environ` 中，未设置的条目保持默认空实现。

### 协程移植

需要为特定架构实现两个协程原语：`coroutine_make` 和 `coroutine_jump`。

```c
/* x64-coroutine.S */
.global x64_coroutine_make
x64_coroutine_make:
    /* 在栈上创建协程上下文，设置入口函数 */
    /* 参数：RDI=stack, RSI=size, RDX=func */
    /* 返回：RAX=context pointer */

.global x64_coroutine_jump
x64_coroutine_jump:
    /* 保存当前上下文，切换到目标上下文 */
    /* 参数：RDI=target_ctx, RSI=priv */
    /* 返回：struct co_transfer_t { fctx, priv } */
    stp x19, x20, [x1], #16
    /* ... 保存/恢复 callee-saved 寄存器 ... */
    ret
```

命名规则为 `<arch>_coroutine_make` 和 `<arch>_coroutine_jump`，汇编文件位于项目的平台目录下。

现有架构实现：
- x64：`projects/x64-linux-sdl-helloworld/linux/x64-coroutine.S`
- ARM64：裸机项目中的 `arm64-coroutine.S`
- RISC-V32/64：对应项目中的 `riscv32-coroutine.S` / `riscv64-coroutine.S`

## 常见开发任务

### 添加新的设备类型

1. 在 `xstar/driver/device.h` 的 `enum device_type_t` 中添加新类型
2. 在 `xstar/driver/device.c` 中更新 `__device_head` 数组大小和类型名称表
3. 创建对应的设备接口头文件（如 `xstar/driver/xxx/xxx.h`）
4. 实现设备注册/搜索函数（如 `register_xxx`、`search_xxx`）
5. 在驱动中使用新设备类型

### 添加新的内核子系统

1. 在 `xstar/kernel/` 下创建子系统目录
2. 定义子系统接口头文件
3. 实现子系统功能
4. 使用适当的 initcall 级别注册
5. 在 `xstar/kernel/Kbuild` 中添加编译规则（目前内核子系统无条件编译）

### 添加新的 LibX 工具函数

1. 在 `xstar/libx/` 目录下创建 `.c` 和 `.h` 文件
2. 实现函数
3. 在 `xstar/libx/Kbuild` 中添加编译规则（目前 LibX 无条件编译）

### 添加新的外部包

1. 在 `packages/` 目录下创建包目录（如 `mypkg-1.0.0/`）
2. 编写 `Kbuild` 文件
3. 在 `packages/Kconfig` 中添加配置选项
4. 在项目 defconfig 中启用包

## 调试技巧

### 使用 GDB

默认编译包含调试信息（`-g -ggdb`），可直接使用 GDB 调试：

```bash
# 启动 GDB
gdb ./projects/<project-name>/output/xstar

# GDB 常用命令
break <function>    # 设置断点
run                 # 运行
next                # 单步执行（跳过函数）
step                # 单步执行（进入函数）
continue            # 继续执行
print <var>         # 打印变量
backtrace           # 查看调用栈
```

### 使用 Shell 命令

XSTAR 提供丰富的 Shell 命令用于调试：

```bash
# 查看设备列表
ls /kobj/device/

# 查看设备信息
cat /kobj/device/framebuffer/fb.0/width

# 查看内存信息
cat /kobj/class/memory/meminfo

# 查看时钟信息
clk

# 查看日期时间
date

# 运行测试
wboxtest -l
wboxtest thread mutex -c 10
```

### 使用日志输出

```c
/* Shell 输出（适用于命令和交互式代码） */
shell_printf("Debug message: %d\n", value);

/* 日志系统（需要 CONFIG_XSTAR_LOG） */
LOG("value = %d\n", value);
```

### 使用 KOBJ 导出调试信息

```c
static ssize_t my_debug_read(struct kobj_t * kobj, void * buf, size_t size)
{
    return xos_snprintf(buf, size, "Debug info: %d\n", my_value);
}

/* 在设备 probe 中创建 KOBJ 调试节点 */
kobj_add_regular(dev->kobj, "debug", my_debug_read, NULL, NULL);
```

KOBJ API 常用函数：

| 函数 | 说明 |
|------|------|
| `kobj_alloc_directory(name)` | 分配目录节点 |
| `kobj_alloc_regular(name, read, write, priv)` | 分配文件节点 |
| `kobj_add(parent, kobj)` | 添加子节点 |
| `kobj_remove(parent, kobj)` | 移除子节点 |
| `kobj_add_directory(parent, name)` | 一步创建并添加目录 |
| `kobj_add_regular(parent, name, read, write, priv)` | 一步创建并添加文件 |
| `kobj_search(parent, name)` | 搜索子节点 |
| `kobj_search_directory_with_create(parent, name)` | 搜索目录，不存在则创建 |

## 最佳实践

### 内存管理

- 始终检查内存分配是否成功
- 在错误路径中释放已分配的内存
- 使用 XOS API（`xos_mem_malloc`/`xos_mem_free`）而非平台原生函数

### 错误处理

- 布尔函数返回 `TRUE`/`FALSE`，指针函数返回 `NULL` 表示失败
- 错误路径中逐层清理已分配的资源
- 设备 probe 失败时释放所有已分配内存后返回 `NULL`

### 可移植性

- 始终使用 XOS API（`xos_io_read32`、`xos_mem_malloc` 等）而非平台原生函数
- 使用 `libx/xdef.h` 中的 `TRUE`/`FALSE`/`NULL` 而非 `<stdbool.h>`
- 使用标准整数类型（`uint32_t` 等）而非平台特定类型

### 代码复用

- 使用 LibX 提供的通用函数（数据结构、算法、加密等）
- 驱动间复用设备类接口（如 `register_clk`、`search_gpiochip`）
- 提取公共代码为独立函数
