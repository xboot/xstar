# Init / Exit Calls (initcall)

`libx/initcall.h` and `libx/initcall.c` provide XSTAR's init/exit call mechanism. Instead of registering initialization code in a central function, a module simply declares an init function with the `xxx_initcall(fn)` macro; the linker automatically collects it into a per-level section, and `xstar_init()` invokes all of them at fixed points during boot. The exit phase is symmetric — `xstar_exit()` calls the functions registered with `xxx_exitcall(fn)` in the reverse order of init.

Inspired by the Linux kernel's `initcall`/`exitcall`, this lets each module keep its startup/teardown code local to its own source file, decoupling registration site from call timing.

## How it works

Each `xxx_initcall(fn)` macro expands to a static pointer constant placed, via the `__section__` attribute, into a link section named `xstar_initcall_<level>`:

```c
#define __define_initcall(level, fn) \
    static const initcall_t __initcall_##level##_##fn \
    __attribute__((__used__, __section__("xstar_initcall_"#level))) = fn
```

Because the section name `xstar_initcall_<level>` is a valid C identifier, binutils automatically emits the boundary symbols `__start_xstar_initcall_<level>` and `__stop_xstar_initcall_<level>` for each section — **no linker-script declaration is needed**. `do_initcalls()` simply walks each section's `[__start_, __stop_)` range and calls every entry.

Exit calls use a fully symmetric set of sections `xstar_exitcall_<level>` and `__start_/__stop_xstar_exitcall_<level>` symbols.

## Init levels

`do_initcalls()` executes the sections in ascending level order, 0 → 9:

| Macro | Level | Purpose |
| --- | --- | --- |
| `pure_initcall` | 0 | Lowest-tier infrastructure with no dependencies. E.g. device/driver frameworks, clocksource/clockevent, spinlock, atomic, timer, font, surface, command framework |
| `machine_initcall` | 1 | Board / machine-specific initialization |
| `core_initcall` | 2 | Core building blocks. E.g. archivers (cpio/sys/dir), audio effect plugins |
| `postcore_initcall` | 3 | Secondary core init after `core` |
| `driver_initcall` | 4 | Concrete device drivers. The most populated level — all peripheral drivers register here |
| `subsys_initcall` | 5 | Subsystem initialization |
| `command_initcall` | 6 | Shell command registration |
| `server_initcall` | 7 | Service / daemon initialization |
| `wboxtest_initcall` | 8 | White-box test registration |
| `late_initcall` | 9 | Latest of the ordinary initcalls |

> Note: `do_initcalls()` is invoked at the **very beginning** of `xstar_init()`, before romdisk, dtree, logger, setting and all other `do_init_*()` steps. So when levels 0–9 run, the filesystem, device tree, logger and settings are not yet available.

### The final level

`final_initcall` / `final_exitcall` is a standalone level — it is **not part of the 0–9 sequence in `do_initcalls()`**, but is driven by the separate `do_init_final()` / `do_exit_final()`:

```c
/*
 * final init calls
 */
#define final_initcall(fn)        __define_initcall(final, fn)
#define final_exitcall(fn)        __define_exitcall(final, fn)
```

Its sections are `xstar_initcall_final` / `xstar_exitcall_final`. `do_init_final()` runs after every `do_init_*()` step has completed and before `do_show_logo()` — by which time romdisk, dtree, logger, setting and the rest of the infrastructure are ready. It is the right hook for "the last beat before entering the main loop".

## Call flow

Init phase (`xstar_init()`, in source order):

```
xos_environ_init(env)
do_initcalls()          # levels 0 → 9
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
do_init_final()         # final level
do_show_logo()
```

Exit phase (`xstar_exit()`) is the reverse of init:

```
xstar_sync()            # flush first (setting, block, ...)
do_exit_final()         # final level exits first (it inited last)
do_exitcalls()          # levels 9 → 0, in reverse
```

Exit is strictly the reverse of init, so resources initialized later are released first.

## Macro reference

| Macro | Description |
| --- | --- |
| `pure_initcall(fn)` … `late_initcall(fn)` | Executed by `do_initcalls()` in level order 0 → 9 |
| `final_initcall(fn)` | Executed by `do_init_final()`, after all `do_init_*()` steps |
| `pure_exitcall(fn)` … `late_exitcall(fn)` | Executed by `do_exitcalls()` in reverse level order 9 → 0 |
| `final_exitcall(fn)` | Executed by `do_exit_final()`, before `do_exitcalls()` |
| `do_initcalls()` / `do_exitcalls()` | Drive levels 0–9 |
| `do_init_final()` / `do_exit_final()` | Drive the final level |

Registered functions have prototype `void fn(void)`, i.e. `initcall_t` / `exitcall_t`:

```c
typedef void (*initcall_t)(void);
typedef void (*exitcall_t)(void);
```

## Examples

Register a device driver (level 4, the most common):

```c
static int foo_probe(...) { ... }

static void foo_driver_init(void)
{
    register_driver(&foo_driver);   /* register with the driver framework */
}
driver_initcall(foo_driver_init);
```

Register a "last beat" routine that must run after the whole system is up, just before the main loop (final level):

```c
static void my_app_start(void)
{
    /* logger, setting, dtree, ... are all available here */
    setting_read("app.autostart", &g_autostart);
}
final_initcall(my_app_start);
```

With a matching exit function when resources need releasing:

```c
static void my_app_stop(void)
{
    /* runs before do_exitcalls() */
}
final_exitcall(my_app_stop);
```

## Usage notes

- The relative order of multiple initcalls **within the same level is not guaranteed** — do not rely on intra-section ordering. If there is a dependency, place the calls in different levels, or synchronize inside the initcall itself.
- Levels 0–9 run **before** the logger is ready; an initcall in those levels **must not** call logging APIs such as `LOG()`. Move logging-dependent init into `final_initcall` or the main loop.
- Initialization that needs romdisk/dtree/setting or other infrastructure should use `final_initcall`, not any of levels 0–9.
- `xxx_exitcall` pairs with the corresponding `xxx_initcall`, and the exit phase always runs in reverse init order — no manual ordering is needed.
- Each level registers an empty sentinel function at the end of `initcall.c` (e.g. `__pure_init`) solely to guarantee the section always exists at link time; application code can ignore them.
