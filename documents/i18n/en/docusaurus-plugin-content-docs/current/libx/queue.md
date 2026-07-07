# Queue (queue)

General-purpose FIFO queue built on list_head_t, with custom cleanup callback support. Not thread-safe; the caller is responsible for ensuring consistency in concurrent scenarios.

## Data Structure

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

- `queue_alloc` — Allocate and initialize a queue. Returns NULL on failure
- `queue_free` — Free the queue. `cb` is called for each remaining node's data (can be NULL)
- `queue_clear` — Remove all nodes from the queue. `cb` is called for each removed node's data (can be NULL)
- `queue_isempty` — Return 1 if the queue is empty, 0 otherwise
- `queue_push` — Push data pointer onto the queue
- `queue_pop` — Pop and return the data pointer from the front of the queue, or NULL if empty
- `queue_peek` — Return the front data pointer without removing it, or NULL if empty

## Example

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
