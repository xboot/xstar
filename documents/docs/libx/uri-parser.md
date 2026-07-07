# URI 解析器 (uri)

URI 字符串解析模块，将 URI 拆分为各个组成部分。

## URI 格式

```
scheme:[//[user[:pass]@]host[:port]][/path][?query][#fragment]
```

示例：

```
https://admin:secret@example.com:8080/api/data?key=value#section
  │       │     │       │           │    │        │          │
scheme   user   pass   host       port  path    query    fragment
```

## 数据结构

```c
struct uri_t {
    char * scheme;    /* 协议名，如 "http"、"ftp" */
    char * user;      /* 用户名 */
    char * pass;      /* 密码 */
    char * host;      /* 主机名或 IP（支持 IPv6 [::1] 格式） */
    int port;         /* 端口号，0 表示未指定 */
    char * path;      /* 路径 */
    char * query;     /* 查询字符串（不含 '?'） */
    char * fragment;  /* 片段标识（不含 '#'） */
};
```

所有字符串字段为独立分配的副本，未出现的部分为 NULL。`port` 未指定时为 0。

## API

```c
struct uri_t * uri_alloc(const char * s);
```

解析 URI 字符串，返回拆分后的 `uri_t` 结构体。解析失败返回 NULL。内部为每个字段分配独立内存。

```c
void uri_free(struct uri_t * uri);
```

释放 uri_t 及其所有字符串字段。

## 解析规则

- **scheme** — 必须以字母开头，可含字母、数字、`+`、`-`、`.`，以 `:` 结束
- **authority** — `//` 开头的部分，包含用户信息、主机和端口
- **user:pass** — `@` 前的部分，`:` 分隔用户名和密码（密码可省略）
- **host** — 支持 IPv6 地址（方括号包裹，如 `[::1]`）
- **port** — 纯数字，范围 0~65535
- **path** — authority 之后、`?` 或 `#` 之前的部分
- **query** — `?` 之后、`#` 之前的部分
- **fragment** — `#` 之后的部分
- 不允许包含控制字符

## 使用示例

### 解析完整 URI

```c
struct uri_t * uri = uri_alloc("https://admin:pass@example.com:8080/api?q=1#top");
/* uri->scheme   = "https"  */
/* uri->user     = "admin"  */
/* uri->pass     = "pass"   */
/* uri->host     = "example.com" */
/* uri->port     = 8080     */
/* uri->path     = "/api"   */
/* uri->query    = "q=1"    */
/* uri->fragment = "top"    */
uri_free(uri);
```

### 解析部分 URI

```c
struct uri_t * uri = uri_alloc("mailto:test@example.com");
/* uri->scheme = "mailto" */
/* uri->path   = "test@example.com" */
/* 其余字段为 NULL 或 0 */
uri_free(uri);
```

### 解析 IPv6 地址

```c
struct uri_t * uri = uri_alloc("http://[::1]:3000/path");
/* uri->host = "[::1]" */
/* uri->port = 3000 */
uri_free(uri);
```
