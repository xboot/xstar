# 队列 (queue)

通用 FIFO 队列，基于 list_head_t 实现，内部无锁，由调用者保证一致性，支持自定义清理回调。

## 数据结构

```c
struct queue_node_t {
    struct list_head_t entry;
    void * data;
};

struct queue_t {
    struct list_head_t head;
};
```

## API

```c
struct queue_t * queue_alloc(void);
void queue_free(struct queue_t * q, void (*cb)(void *));
void queue_clear(struct queue_t * q, void (*cb)(void *));
int queue_isempty(struct queue_t * q);
void queue_push(struct queue_t * q, void * data);
void * queue_pop(struct queue_t * q);
void * queue_peek(struct queue_t * q);
```

- `queue_alloc` — 创建空队列，失败返回 NULL
- `queue_free` — 销毁队列，cb 为每个节点 data 的清理回调（可为 NULL）
- `queue_clear` — 清空队列，cb 为每个节点 data 的清理回调（可为 NULL）
- `queue_isempty` — 队列为空返回 1，否则返回 0
- `queue_push` — 入队
- `queue_pop` — 出队，空时返回 NULL
- `queue_peek` — 查看队首元素（不移除），空时返回 NULL

## 使用示例

```c
static void data_cleanup(void * data)
{
    if(data)
        xos_mem_free(data);
}

struct queue_t * q = queue_alloc();

queue_push(q, xos_strdup("first"));
queue_push(q, xos_strdup("second"));
queue_push(q, xos_strdup("third"));

if(!queue_isempty(q))
{
    void * data = queue_pop(q);
    printf("%s\n", (char *)data);
    xos_mem_free(data);
}

queue_free(q, data_cleanup);
```
