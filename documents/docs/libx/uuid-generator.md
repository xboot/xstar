# UUID 生成器 (uuid)

生成符合 RFC 4122 第4版的 UUID 字符串。

## 格式

```
xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
```

- 版本号固定为 `4`（第3段首字符），表示随机生成
- 变体标识 `y` 为 `8`、`9`、`a`、`b` 之一（第4段首字符），符合 RFC 4122 变体规范
- 其余字符为随机十六进制，共 36 字符（含 4 个连字符）

示例：`f47ac10b-58cc-4372-a567-0e02b2c3d479`

## API

```c
const char * uuid4(char * buf);
```

生成并返回 UUID v4 字符串指针。

- 若 `buf` 为 `NULL`，使用内部静态缓冲区，每次调用会覆盖上一次的结果
- 若 `buf` 非 `NULL`，写入用户提供的缓冲区（至少 37 字节），结果可安全保留

> 当 `buf` 为 `NULL` 时非线程安全，不可在多线程中并发调用。

## 使用示例

### 生成唯一标识

```c
const char * id = uuid4(NULL);
/* id 指向内部静态缓冲区，下次调用会失效 */
```

### 保留 UUID 值

```c
char buf[37];
const char * id = uuid4(buf);
/* buf 中保存了 UUID 值，可安全保留 */
```

### 动态字符串保存

```c
struct ds_t * ds = ds_alloc();
ds_copy(ds, "%s", uuid4(NULL));
/* ds 中保存了 UUID 值，不受后续 uuid4 调用影响 */
ds_free(ds);
```
