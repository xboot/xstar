# 动态字符串 (ds)

动态字符串库，提供自动扩容的字符串操作，包括追加、插入、删除、查找、替换、分割等。

## 数据结构

```c
struct ds_t {
    char * data;       /* 以 '\0' 结尾的字符串数据 */
    size_t length;     /* 字符串长度（不含 '\0'） */
    size_t capacity;   /* 当前容量 */
};
```

## API

### 分配与释放

```c
struct ds_t * ds_alloc(void);
```

分配空字符串，初始容量 16。

```c
struct ds_t * ds_alloc_from_str(const char * str);
```

从 C 字符串创建，复制内容。

```c
struct ds_t * ds_alloc_from_buf(const void * buf, int len);
```

从指定缓冲区创建，复制 `len` 字节内容，并自动追加 `'\0'` 结尾。`buf` 为 NULL 或 `len <= 0` 时返回空串。

```c
void ds_free(struct ds_t * ds);
```

释放字符串及结构体。

### 访问器

```c
size_t ds_length(struct ds_t * ds);    /* 字符串长度 */
size_t ds_capacity(struct ds_t * ds);  /* 当前容量 */
int ds_isempty(struct ds_t * ds);      /* 是否为空，空返回 1 */
const char * ds_cstr(struct ds_t * ds); /* C 字符串指针，NULL 安全，返回 "" */
```

### 复制与子串

```c
struct ds_t * ds_clone(struct ds_t * ds);
```

深拷贝，返回新的 ds_t。

```c
struct ds_t * ds_substr(struct ds_t * ds, size_t start, size_t len);
```

取子串，返回新的 ds_t。`start` 超出长度时返回空串，`len` 超出范围时自动截断。

### 清空与修剪

```c
void ds_clear(struct ds_t * ds);
```

清空字符串（length 归零，不释放内存）。

```c
void ds_trim(struct ds_t * ds);
```

去除首尾空白字符。

### 追加与前插

```c
int ds_append(struct ds_t * ds, const char * str);
int ds_append_char(struct ds_t * ds, const char c);
int ds_prepend(struct ds_t * ds, const char * str);
```

成功返回 1，失败返回 0。

### 插入与删除

```c
int ds_insert(struct ds_t * ds, size_t pos, const char * str);
```

在 `pos` 位置插入字符串，`pos` 超出长度时追加到末尾。

```c
int ds_delete(struct ds_t * ds, size_t start, size_t len);
```

从 `start` 位置删除 `len` 个字符，范围越界时自动截断。

### 格式化

```c
int ds_copy(struct ds_t * ds, const char * fmt, ...);
```

用 printf 风格格式化**替换**整个字符串内容。返回写入的字符数，失败返回 0。

```c
int ds_concat(struct ds_t * ds, const char * fmt, ...);
```

用 printf 风格格式化**追加**到末尾。返回追加的字符数，失败返回 0。

### 查找

```c
int ds_find(struct ds_t * ds, const char * sub);
```

从左查找子串，返回位置索引，未找到返回 -1。空子串返回 length。

```c
int ds_rfind(struct ds_t * ds, const char * sub);
```

从右查找子串最后一次出现的位置，未找到返回 -1。空子串返回 length。

### 替换

```c
int ds_replace(struct ds_t * ds, const char * ostr, const char * nstr);
```

替换所有匹配的子串，返回替换次数。

```c
int ds_replace_first(struct ds_t * ds, const char * ostr, const char * nstr);
int ds_replace_last(struct ds_t * ds, const char * ostr, const char * nstr);
```

分别替换第一个/最后一个匹配。成功返回 1，未找到返回 0。

### 分割迭代

```c
struct ds_iter_t * ds_iter_alloc(struct ds_t * ds, const char * delim);
void ds_iter_free(struct ds_iter_t * iter);
char * ds_iter_next(struct ds_iter_t * iter);
```

按分隔符分割字符串，迭代获取每个 token。`ds_iter_next` 返回 NULL 表示结束。

> 注意：迭代器内部复制了原始字符串数据，修改原 ds_t 不影响迭代。返回的 token 指针在 `ds_iter_free` 后失效。

## 使用示例

### 基本操作

```c
struct ds_t * s = ds_alloc_from_str("hello");
ds_append(s, " world");
ds_prepend(s, ">> ");
/* ds_cstr(s) == ">> hello world" */
ds_free(s);
```

### 格式化

```c
struct ds_t * s = ds_alloc();
ds_copy(s, "count: %d", 42);
ds_concat(s, ", name: %s", "test");
/* ds_cstr(s) == "count: 42, name: test" */
ds_free(s);
```

### 查找与替换

```c
struct ds_t * s = ds_alloc_from_str("aabbccaabb");
ds_replace(s, "aa", "XX");        /* "XXbbccXXbb", 返回 2 */
ds_replace_first(s, "bb", "YY");  /* "XXYYccXXbb", 返回 1 */
ds_free(s);
```

### 分割迭代

```c
struct ds_t * s = ds_alloc_from_str("one,two,three");
struct ds_iter_t * iter = ds_iter_alloc(s, ",");
char * token;
while((token = ds_iter_next(iter)) != NULL)
{
    /* 依次: "one", "two", "three" */
}
ds_iter_free(iter);
ds_free(s);
```
