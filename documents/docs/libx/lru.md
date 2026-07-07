# LRU缓存 (lru)

最近最少使用（LRU）缓存，基于哈希表 + 双向链表实现，超容量时淘汰最久未访问的条目，支持淘汰回调。访问时自动提升热点条目到链表头部。

## API

```c
struct lru_t * lru_alloc(size_t maxbytes, unsigned int hashpower, void (*cb)(struct lru_t *, const char *, int, void *, int));
void lru_free(struct lru_t * l);
int lru_get(struct lru_t * l, const char * key, int nkey, void * buf, int nbuf);
int lru_set(struct lru_t * l, const char * key, int nkey, void * buf, int nbuf);
int lru_remove(struct lru_t * l, const char * key, int nkey);
```

- `lru_alloc` — 创建缓存。maxbytes为0时默认64KB；hashpower为0时根据maxbytes自动推算（每桶约64字节，最小6最大20）；cb为淘汰回调（可NULL）
- `lru_free` — 销毁缓存及所有条目
- `lru_get` — 按key获取值，命中时自动提升到链表头部。返回拷贝字节数，未命中返回0
- `lru_set` — 设置key-value，超容量时从链表尾部淘汰。返回写入字节数，失败返回0
- `lru_remove` — 删除条目。成功返回1，未找到返回0

## 使用示例

```c
struct lru_t * cache = lru_alloc(4096, 0, NULL);

char * key = "hello";
char * val = "world";
lru_set(cache, key, strlen(key), val, strlen(val) + 1);

char buf[64];
int n = lru_get(cache, key, strlen(key), buf, sizeof(buf));
if (n > 0)
	printf("%s\n", buf);

lru_remove(cache, key, strlen(key));
lru_free(cache);
```
