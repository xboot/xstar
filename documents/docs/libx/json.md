# JSON解析 (json)

JSON 解析器,将 JSON 文本解析为 `json_value_t` 节点树,支持所有标准 JSON 类型(对象、数组、整数、浮点、字符串、布尔、null)。

解析器特性:
- 单文件、零外部依赖,采用两遍扫描(先测量、再分配),整棵树的内存分配次数为 O(N) 级别
- 整数使用 `int64_t`,溢出时自动升级为 `double`
- 完整支持 `\uXXXX` 转义、UTF-16 代理对、UTF-8 输出
- 自动跳过 UTF-8 BOM(`EF BB BF`)
- 允许 C 风格注释 `//` 和 `/* */`(非标准 JSON 扩展)
- 输入按 `length` 字节读取,允许内嵌 `\0`,无须以 NUL 结尾

## 类型

```c
enum json_type_t {
    JSON_NONE,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_INTEGER,
    JSON_DOUBLE,
    JSON_STRING,
    JSON_BOOLEAN,
    JSON_NULL
};

struct json_value_t {
    struct json_value_t * parent;
    enum json_type_t type;
    union {
        int boolean;
        int64_t integer;
        double dbl;
        struct { unsigned int length; char * ptr; } string;
        struct { unsigned int length; struct json_object_entry_t * values; } object;
        struct { unsigned int length; struct json_value_t ** values; } array;
    } u;
    /* reserved 字段供解析器内部使用,业务代码不要访问 */
};

struct json_object_entry_t {
    char * name;
    unsigned int name_length;
    struct json_value_t * value;
};
```

## API

```c
struct json_value_t * json_parse(const char * json, size_t length, char * errbuf);
void json_free(struct json_value_t * value);
```

- `json_parse` — 解析 JSON 文本,成功返回根节点,失败返回 `NULL`。
  `errbuf` 用于接收错误信息,可传 `NULL` 表示不需要;若传入则**至少要预留 256 字节**。
  错误信息形如 `line 12: Unexpected ',' in object`(只精确到行号)。
- `json_free` — 释放整棵解析树,迭代实现,不会因深度嵌套导致 C 栈溢出。

## 使用示例

```c
char errbuf[256];
const char * text = "{\"name\":\"xstar\",\"version\":1}";
struct json_value_t * root = json_parse(text, strlen(text), errbuf);

if(root && root->type == JSON_OBJECT)
{
    for(unsigned int i = 0; i < root->u.object.length; i++)
    {
        struct json_object_entry_t * entry = &root->u.object.values[i];
        if(entry->value->type == JSON_STRING)
            printf("%s = %s\n", entry->name, entry->value->u.string.ptr);
        else if(entry->value->type == JSON_INTEGER)
            printf("%s = %lld\n", entry->name, (long long)entry->value->u.integer);
    }
}
else if(!root)
{
    printf("parse error: %s\n", errbuf);
}

json_free(root);
```
