# 初始化与退出调用 (initcall)

`libx/initcall.h` 与 `libx/initcall.c` 提供了 XSTAR 的初始化/退出调用机制。模块无需在某个中心函数里手工登记自己的初始化代码，只需用 `xxx_initcall(fn)` 宏声明一个初始化函数，链接器会自动把它收集到对应等级的专属段中，再由 `xstar_init()` 在启动流程的固定时机统一调用。退出阶段同理，由 `xstar_exit()` 按 init 的逆序调用 `xxx_exitcall(fn)` 登记的函数。

该机制借鉴自 Linux 内核的 `initcall`/`exitcall`，使各模块的启动/收尾代码可以就近写在各自的源文件里，解耦注册顺序与调用时序。

## 工作原理

每个 `xxx_initcall(fn)` 宏展开为一个静态指针常量，并通过 `__section__` 属性放进名为 `xstar_initcall_<level>` 的链接段：

```c
#define __define_initcall(level, fn) \
    static const initcall_t __initcall_##level##_##fn \
    __attribute__((__used__, __section__("xstar_initcall_"#level))) = fn
```

由于段名 `xstar_initcall_<level>` 是合法的 C 标识符，binutils 会自动为每个段生成 `__start_xstar_initcall_<level>` 与 `__stop_xstar_initcall_<level>` 两个边界符号，**无需在链接脚本里手工声明**。`do_initcalls()` 即遍历各段的 `[__start_, __stop_)` 区间逐个调用。

退出调用走完全对称的另一组段 `xstar_exitcall_<level>` 与 `__start_/__stop_xstar_exitcall_<level>` 符号。

## 初始化等级

`do_initcalls()` 内部按等级 0→9 的顺序依次执行各段：

| 宏 | 等级 | 用途 |
| --- | --- | --- |
| `pure_initcall` | 0 | 最底层基础设施，无任何依赖。如设备/驱动框架、时钟源/事件、自旋锁、原子操作、定时器、字体、surface、命令框架等 |
| `machine_initcall` | 1 | 板级/机器相关的初始化 |
| `core_initcall` | 2 | 核心构建块。如归档器（cpio/sys/dir）、各类音频 effect 插件 |
| `postcore_initcall` | 3 | core 之后的次级核心初始化 |
| `driver_initcall` | 4 | 具体设备驱动。这是数量最多的一档，所有外设驱动都登记在此 |
| `subsys_initcall` | 5 | 子系统初始化 |
| `command_initcall` | 6 | shell 命令注册 |
| `server_initcall` | 7 | 服务/守护初始化 |
| `wboxtest_initcall` | 8 | 白盒测试登记 |
| `late_initcall` | 9 | 最晚的普通初始化 |

> 注意：`do_initcalls()` 在 `xstar_init()` 的**最开头**被调用，先于 romdisk、dtree、logger、setting 等所有 `do_init_*()` 步骤。因此等级 0~9 的 initcall 执行时，文件系统、设备树、日志器、设置系统等都尚未就绪。

### final 等级

`final_initcall` / `final_exitcall` 是独立的一档，**不属于 `do_initcalls()` 的 0~9 序列**，而由单独的 `do_init_final()` / `do_exit_final()` 驱动：

```c
/*
 * final init calls
 */
#define final_initcall(fn)        __define_initcall(final, fn)
#define final_exitcall(fn)        __define_exitcall(final, fn)
```

它对应的段为 `xstar_initcall_final` / `xstar_exitcall_final`。`do_init_final()` 在所有 `do_init_*()` 步骤完成之后、`do_show_logo()` 之前执行，此时 romdisk、dtree、logger、setting 等基础设施均已就绪，是“进入主循环前的最后一拍”的合适挂载点。

## 调用流程

初始化阶段（`xstar_init()`，按源码顺序）：

```
xos_environ_init(env)
do_initcalls()          # 等级 0 → 9 全部执行
do_init_romdisk()
do_init_dtree()
do_init_wallclock()
do_init_memory()
do_init_logger()
do_init_version()
do_init_copyright()
do_init_random()
do_init_feature()
do_init_font()
do_init_setting()
do_init_final()         # final 等级
do_show_logo()
```

退出阶段（`xstar_exit()`）按 init 的逆序：

```
xstar_sync()            # 先同步落盘（setting、block 等）
do_exit_final()         # final 等级最先退出（因为它最晚 init）
do_exitcalls()          # 等级 9 → 0 逆序执行
```

退出与初始化严格反序，保证后初始化的资源先被释放。

## 宏一览

| 宏 | 说明 |
| --- | --- |
| `pure_initcall(fn)` … `late_initcall(fn)` | 在 `do_initcalls()` 中按等级 0→9 执行 |
| `final_initcall(fn)` | 在 `do_init_final()` 中执行，位于全部 `do_init_*()` 之后 |
| `pure_exitcall(fn)` … `late_exitcall(fn)` | 在 `do_exitcalls()` 中按等级 9→0 逆序执行 |
| `final_exitcall(fn)` | 在 `do_exit_final()` 中执行，先于 `do_exitcalls()` |
| `do_initcalls()` / `do_exitcalls()` | 驱动 0~9 等级 |
| `do_init_final()` / `do_exit_final()` | 驱动 final 等级 |

登记的函数原型均为 `void fn(void)`，即 `initcall_t` / `exitcall_t`：

```c
typedef void (*initcall_t)(void);
typedef void (*exitcall_t)(void);
```

## 使用示例

登记一个设备驱动（等级 4，最常用的一档）：

```c
static int foo_probe(...) { ... }

static void foo_driver_init(void)
{
    register_driver(&foo_driver);   /* 注册到驱动框架 */
}
driver_initcall(foo_driver_init);
```

登记一个需要在系统全部就绪后、进入主循环前执行的“最后一拍”逻辑（final 等级）：

```c
static void my_app_start(void)
{
    /* 此时 logger、setting、dtree 等均已可用 */
    setting_read("app.autostart", &g_autostart);
}
final_initcall(my_app_start);
```

对应的退出函数（如有资源需要释放）：

```c
static void my_app_stop(void)
{
    /* 在 do_exitcalls() 之前执行 */
}
final_exitcall(my_app_stop);
```

## 使用建议

- 同一等级内多个 initcall 的相对顺序不保证，**不要依赖同段内的先后顺序**；若存在依赖，应把它们放到不同等级，或在 initcall 内部自行同步。
- 等级 0~9 在 logger 就绪**之前**执行，initcall 内部**不应**调用 `LOG()` 之类的日志接口；需要日志的初始化逻辑放到 `final_initcall` 或主循环中。
- 需要访问 romdisk/dtree/setting 等基础设施的初始化，应使用 `final_initcall`，而非 0~9 任意等级。
- `xxx_exitcall` 与对应 `xxx_initcall` 成对出现，且退出阶段总是按 init 逆序执行，无需手动排序。
- 每个等级在 `initcall.c` 末尾都登记了一个空的哨兵函数（如 `__pure_init`），仅用于保证该段在链接时一定存在，应用代码无需关注。
