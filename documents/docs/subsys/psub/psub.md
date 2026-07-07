# 发布订阅 (psub)

系统级的发布订阅消息总线，基于主题（topic）的松耦合通信机制，使模块间无需直接引用即可交换数据，实现生产者与消费者的完全解耦。

## 解耦作用

传统模块间通信通常通过直接函数调用，导致模块间产生编译时依赖和运行时耦合：

- **直接调用**：模块 A 调用模块 B 的函数，A 必须包含 B 的头文件，且 B 必须先于 A 初始化
- **发布订阅**：模块 A 发布消息到主题 `"topic-x"`，模块 B 订阅 `"topic-x"`，A 和 B 互不感知，无需包含对方头文件，初始化顺序无关

这种解耦带来以下优势：

- **消除依赖**：发布者和订阅者之间无编译时依赖，各自独立编译和演进
- **一对多通信**：一个发布事件可同时通知多个订阅者，无需发布者维护订阅者列表
- **动态订阅**：订阅者可在运行时按需订阅和取消订阅，灵活响应系统状态变化
- **生命周期解耦**：发布时若无订阅者，事件自动丢弃；订阅后若有新发布，立即收到通知
- **一次性订阅**：通过 `oneshot` 标志支持"订阅一次后自动取消"模式，适用于初始化等待场景

## 工作原理

### 内部结构

`psub` 是 `libx/ps` 的系统级封装，在 `pure_initcall` 阶段创建全局 `psctx_t` 上下文：

```
psctx_t (全局上下文)
  ├── 哈希表 (主题索引)
  │   ├── "wifi.connected" → pstopic_t
  │   │   ├── pscb_t (订阅者1: cb1, sdat1)
  │   │   └── pscb_t (订阅者2: cb2, sdat2)
  │   └── "sensor.data" → pstopic_t
  │       └── pscb_t (订阅者3: cb3, sdat3)
  └── 互斥锁
```

### 主题管理

- 主题为字符串，通过字符串哈希（`shash`）索引
- 哈希表自动扩容：当主题数量超过哈希表容量的一半时，容量翻倍
- 主题在首次订阅时自动创建，无订阅者时自动保留（不主动删除）

### 发布流程

`psub_publish()` 的处理过程：

1. 在哈希表中查找主题，不存在则直接返回（事件丢弃）
2. 将订阅者回调列表splice到本地链表（脱离锁保护）
3. 逐个调用订阅者回调：`cb(pdat, sdat)`
4. 回调执行完毕后，重新加锁，将非 oneshot 的订阅者放回列表
5. 释放 oneshot 订阅者的内存

这种"先 splice 再回调"的设计避免了回调中再次发布/订阅同一主题时的死锁。

### 订阅去重

同一主题上，若已存在 `cb` 和 `sdat` 完全相同的订阅，则忽略新的订阅请求，防止重复回调。

## API

| 函数 | 说明 |
|------|------|
| `psub_publish(topic, pdat)` | 向主题发布消息，`pdat` 为传递给订阅者的数据指针 |
| `psub_subscribe(topic, cb, sdat, oneshot)` | 订阅主题，`cb` 为回调函数，`sdat` 为订阅者私有数据，`oneshot` 为 1 时触发一次后自动取消 |
| `psub_unsubscribe(topic, cb, sdat)` | 取消订阅，`cb` 为 NULL 时取消该主题下所有订阅 |

回调函数原型：`void cb(void * pdat, void * sdat)`

- `pdat`：发布者传递的数据
- `sdat`：订阅者注册时的私有数据

## 用法示例

### 基本发布订阅

```c
#include <kernel/core/psub.h>

/* 订阅者 */
static void on_wifi_connected(void * pdat, void * sdat)
{
    const char * ssid = (const char *)pdat;
    struct my_app_t * app = (struct my_app_t *)sdat;
    LOG("wifi connected: %s\n", ssid);
    app->wifi_ready = 1;
}

void app_init(void)
{
    struct my_app_t * app = get_app();
    psub_subscribe("wifi.connected", on_wifi_connected, app, 0);
}

/* 发布者（另一个模块，无需知道 app 的存在） */
void wifi_driver_notify(void)
{
    psub_publish("wifi.connected", "MyNetwork");
}
```

### 一次性订阅（初始化等待）

```c
/* 等待某个服务就绪，只通知一次 */
static void on_service_ready(void * pdat, void * sdat)
{
    LOG("service is ready, starting dependent module\n");
    start_my_module();
}

void init_dependent_module(void)
{
    /* oneshot = 1，触发后自动取消订阅并释放内存 */
    psub_subscribe("service.ready", on_service_ready, NULL, 1);
}
```

### 取消订阅

```c
/* 精确取消 */
psub_unsubscribe("sensor.data", my_callback, my_data);

/* 取消该主题下所有订阅 */
psub_unsubscribe("sensor.data", NULL, NULL);
```

### 多订阅者场景

```c
/* 显示模块：订阅电池电量更新 UI */
static void on_battery_update(void * pdat, void * sdat)
{
    int level = *(int *)pdat;
    ui_set_battery_level(level);
}
psub_subscribe("battery.level", on_battery_update, NULL, 0);

/* 日志模块：订阅电池电量记录日志 */
static void on_battery_log(void * pdat, void * sdat)
{
    int level = *(int *)pdat;
    LOG("battery: %d%%\n", level);
}
psub_subscribe("battery.level", on_battery_log, NULL, 0);

/* 电源驱动：发布一次，两个订阅者同时收到 */
int level = 85;
psub_publish("battery.level", &level);
```

## 说明

- `psub` 在 `pure_initcall()` 阶段自动初始化全局上下文，无需手动调用
- 所有操作通过互斥锁保护，线程安全
- 发布时若无订阅者，事件被静默丢弃，不产生错误
- 回调在发布者线程中同步执行，回调内应避免耗时操作
- 回调内可以安全地调用 `psub_publish()` 或 `psub_subscribe()`（不会死锁）
- `oneshot` 订阅触发后自动释放内存，无需手动取消
- `psub_unsubscribe()` 中 `cb` 传 NULL 可取消该主题下所有订阅
- 底层基于 `libx/ps` 实现，支持独立的 `psctx_t` 上下文用于隔离不同发布订阅域
