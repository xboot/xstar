# 延时回调 (delaycall)

基于定时器系统的延时回调机制，在指定毫秒数后异步执行回调函数，无需手动管理定时器生命周期。

## 工作原理

`delaycall()` 内部分配一个 `delaycall_pdat_t` 结构体，封装定时器和回调信息：

1. 分配 `delaycall_pdat_t`，保存用户回调函数和数据
2. 初始化内部定时器，回调函数为 `delaycall_timer_function`
3. 调用 `timer_start()` 启动定时器，间隔为指定的毫秒数
4. 定时器到期后，`delaycall_timer_function` 执行用户回调并自动释放内存

整个过程无需手动调用 `timer_init()` 或 `timer_cancel()`，定时器到期后自动清理。

## API

| 函数 | 说明 |
|------|------|
| `delaycall(ms, func, data)` | 延时 `ms` 毫秒后异步执行 `func(data)` |

## 用法示例

```c
#include <kernel/time/delaycall.h>

static void on_led_off(void * data)
{
    int * gpio = (int *)data;
    led_set(*gpio, 0);  /* 延时后关闭 LED */
}

void flash_led(int gpio)
{
    led_set(gpio, 1);           /* 点亮 LED */
    delaycall(500, on_led_off, &gpio);  /* 500ms 后自动关闭 */
}
```

### 延时清理资源

```c
static void cleanup_resource(void * data)
{
    struct my_ctx_t * ctx = (struct my_ctx_t *)data;
    xos_mem_free(ctx->buffer);
    xos_mem_free(ctx);
}

void deferred_cleanup(struct my_ctx_t * ctx)
{
    /* 100ms 后异步清理，让当前操作先完成 */
    delaycall(100, cleanup_resource, ctx);
}
```

## 说明

- 延时回调基于定时器系统（`timer`）实现，精度取决于时钟事件设备
- 回调在定时器中断上下文中执行，应避免耗时操作
- `delaycall()` 内部自动分配和释放内存，调用方无需管理
- `ms` 为 0 或 `func` 为 NULL 时，函数直接返回不执行任何操作
- 回调函数执行完毕后，内部定时器和 `delaycall_pdat_t` 自动释放
