# 发布订阅系统 (psub)

XSTAR 的发布订阅（Publish-Subscribe）机制，实现模块间的松耦合通信。发布者按主题发出消息，订阅该主题的回调自动被调用，发布者与订阅者互不感知。

## 架构

```
发布者                    订阅者
psub_publish("net/status", data)  →  cb1(data, sdat1)
                                   →  cb2(data, sdat2)
                                   →  ...（所有订阅该主题的回调）
```

系统启动时通过 `pure_initcall` 创建全局上下文，提供 `psub_*` 全局便捷接口。底层实现 `ps_*` 支持创建独立的上下文实例。

## 两层接口

### 全局接口 (psub)

使用系统全局唯一的发布订阅上下文，简单直接，适用于大多数场景。

```c
void psub_publish(const char * topic, void * pdat);
void psub_subscribe(const char * topic, void (*cb)(void *, void *), void * sdat, int oneshot);
void psub_unsubscribe(const char * topic, void (*cb)(void *, void *), void * sdat);
```

### 实例接口 (ps)

可创建独立的发布订阅上下文，适用于需要隔离的场景（如不同子系统各自管理）。

```c
struct psctx_t * ps_alloc(int size);
void ps_free(struct psctx_t * ctx);
void ps_publish(struct psctx_t * ctx, const char * topic, void * pdat);
void ps_subscribe(struct psctx_t * ctx, const char * topic, void (*cb)(void *, void *), void * sdat, int oneshot);
void ps_unsubscribe(struct psctx_t * ctx, const char * topic, void (*cb)(void *, void *), void * sdat);
```

实例接口的 `ps_subscribe` 以 `(cb, sdat)` 组合作为唯一标识，同一回调可配合不同 `sdat` 多次订阅同一主题；`ps_unsubscribe` 按 `(cb, sdat)` 精确匹配删除对应订阅。全局接口 `psub_*` 与之语义一致。

## API 详解

### 发布

```c
void psub_publish(const char * topic, void * pdat);
```

向 `topic` 发布消息，`pdat` 为发布者传递的数据指针。所有订阅该主题的回调按订阅顺序被调用。若主题无订阅者则无操作。

### 订阅

```c
void psub_subscribe(const char * topic, void (*cb)(void *, void *), void * sdat, int oneshot);
```

订阅 `topic`，`cb` 为回调函数，`sdat` 为订阅者私有数据。订阅以 `(cb, sdat)` 组合作为唯一标识，同一 `cb` 可配合不同 `sdat` 多次订阅同一主题；若 `(cb, sdat)` 已存在，则重复订阅被忽略。

- `oneshot = 0` — 持续订阅，每次发布都触发
- `oneshot = 1` — 一次性订阅，触发一次后自动取消

回调签名：`void cb(void * pdat, void * sdat)`
- `pdat` — 发布者传入的数据
- `sdat` — 订阅时传入的私有数据

### 取消订阅

```c
void psub_unsubscribe(const char * topic, void (*cb)(void *, void *), void * sdat);
```

取消 `topic` 上的订阅，支持两种匹配模式：

- `cb != NULL` — 按 `(cb, sdat)` 精确匹配，仅取消该回调对应的订阅。`sdat` 必须与订阅时传入的值一致，否则不会删除。
- `cb == NULL` — 通配匹配，取消该 `topic` 下的**全部**订阅（此时 `sdat` 参数被忽略）。

主题本身不会被删除，仍保留在上下文中以备后续订阅，直至 `ps_free` 时统一回收。

## 自动清理

- 一次性订阅触发后自动移除
- 上下文哈希表按需自动扩容
- 主题创建后保留至 `ps_free` 时统一释放（不会因订阅者清空而单独回收）

## 使用示例

### 基本发布订阅

```c
static void on_network_status(void * pdat, void * sdat)
{
    int * status = (int *)pdat;
    printf("network status: %d\n", *status);
}

psub_subscribe("net/status", on_network_status, NULL, 0);

int status = 1;
psub_publish("net/status", &status);
/* on_network_status 被调用 */
```

### 带私有数据

```c
struct my_app_t {
    int id;
};

static void on_data(void * pdat, void * sdat)
{
    struct my_app_t * app = (struct my_app_t *)sdat;
    char * data = (char *)pdat;
    printf("app%d received: %s\n", app->id, data);
}

struct my_app_t app = { .id = 42 };
psub_subscribe("data/update", on_data, &app, 0);
```

### 一次性订阅

```c
static void on_init_done(void * pdat, void * sdat)
{
    printf("initialization complete\n");
}

psub_subscribe("system/init", on_init_done, NULL, 1);
psub_publish("system/init", NULL);   /* 触发并自动取消 */
psub_publish("system/init", NULL);   /* 不再触发 */
```

### 独立上下文

```c
struct psctx_t * my_ps = ps_alloc(16);

ps_subscribe(my_ps, "event/a", my_callback, NULL, 0);
ps_publish(my_ps, "event/a", data);

ps_free(my_ps);
```

### 批量取消订阅

```c
psub_subscribe("net/status", cb1, &ctx1, 0);
psub_subscribe("net/status", cb2, &ctx2, 0);
psub_subscribe("net/status", cb3, NULL, 0);

/* 精确取消单个订阅 */
psub_unsubscribe("net/status", cb1, &ctx1);

/* 通配清空该主题剩余的所有订阅 */
psub_unsubscribe("net/status", NULL, NULL);
```
