# SHA-1哈希 (sha1)

SHA-1 安全哈希算法，160 位摘要计算。提供一次性计算和流式更新两种接口。

## 常量与数据结构

```c
#define SHA1_DIGEST_SIZE (20)

struct sha1_ctx_t {
    uint64_t count;
    uint8_t buf[64];
    uint32_t state[5];
};
```

## API

### 一次性计算

```c
const uint8_t * sha1_hash(const void * data, int len, uint8_t * digest);
```

- `data` — 输入数据
- `len` — 数据长度
- `digest` — 输出 20 字节摘要

返回 `digest` 指针。

### 流式更新

```c
void sha1_init(struct sha1_ctx_t * ctx);
void sha1_update(struct sha1_ctx_t * ctx, const void * data, int len);
const uint8_t * sha1_final(struct sha1_ctx_t * ctx);
```

- `sha1_init` — 初始化上下文
- `sha1_update` — 追加数据，可多次调用
- `sha1_final` — 完成计算，返回摘要指针（指向上下文内部缓冲区）

## 使用示例

### 一次性计算

```c
uint8_t msg[] = { 'x', 'b', 'o', 'o', 't' };
uint8_t digest[SHA1_DIGEST_SIZE];
sha1_hash(msg, sizeof(msg), digest);
/* digest = 0x7182df07 c7f20675 0fe2ed98 580ab526 cb839f5d */
```

### 流式更新

```c
struct sha1_ctx_t ctx;
uint8_t digest[SHA1_DIGEST_SIZE];

sha1_init(&ctx);
sha1_update(&ctx, chunk1, len1);
sha1_update(&ctx, chunk2, len2);
sha1_final(&ctx);
```
