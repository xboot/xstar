# 动态缓冲区 (db)

动态字节缓冲区，用于二进制数据的自动扩容管理。容量按 2 的幂次增长，适合协议解析、数据拼装等场景。

## 数据结构

```c
struct db_t {
    unsigned char * data;   /* 数据指针 */
    size_t length;          /* 当前数据长度 */
    size_t capacity;        /* 当前容量 */
};
```

## API

### 分配与释放

```c
struct db_t * db_alloc(size_t size);
```

分配缓冲区，`size` 为预分配容量（最小 32，自动向上取整到 2 的幂）。返回 NULL 表示失败。

```c
void db_free(struct db_t * db);
```

释放缓冲区及结构体。

### 访问器

```c
size_t db_length(struct db_t * db);     /* 数据长度 */
size_t db_capacity(struct db_t * db);   /* 当前容量 */
unsigned char * db_data(struct db_t * db); /* 数据指针 */
```

传入 NULL 安全，返回 0 或 NULL。

### 操作

```c
void db_clear(struct db_t * db);
```

清空数据（length 归零，不释放内存）。

```c
int db_append(struct db_t * db, const void * buf, size_t len);
int db_append_byte(struct db_t * db, unsigned char byte);
```

尾部追加数据。成功返回 1，失败返回 0。

```c
int db_prepend(struct db_t * db, const void * buf, size_t len);
int db_prepend_byte(struct db_t * db, unsigned char byte);
```

头部插入数据（原有数据后移）。成功返回 1，失败返回 0。

```c
const char * db_safe_cstr(struct db_t * db);
```

在数据末尾临时写入 `\0`，返回可安全用作 C 字符串的只读指针。不改变 length。失败返回空字符串 `""`。

> 注意：后续的 append/prepend 操作可能使返回的指针失效。

## 扩容策略

容量不足时按 2 倍增长，直到满足需求。初始容量自动取整到 2 的幂（最小 32）。

## 使用示例

```c
struct db_t * db = db_alloc(64);

db_append(db, "hello", 5);
db_append_byte(db, ' ');
db_append(db, "world", 5);
db_prepend(db, ">> ", 3);

/* 当作字符串使用 */
const char * s = db_safe_cstr(db);  /* ">> hello world" */

db_free(db);
```
