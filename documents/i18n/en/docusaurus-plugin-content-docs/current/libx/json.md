# JSON Parser (json)

JSON parser that parses JSON text into a tree of `json_value_t` nodes, supporting all standard JSON types (object, array, integer, double, string, boolean, null).

Parser characteristics:
- Single file, no external dependencies. Uses two-pass scanning (measure first, then allocate); the total number of memory allocations is O(N) in the node count.
- Integers use `int64_t`; on overflow they are automatically promoted to `double`.
- Full support for `\uXXXX` escapes, UTF-16 surrogate pairs, and UTF-8 output.
- UTF-8 BOM (`EF BB BF`) is skipped automatically.
- C-style comments `//` and `/* */` are accepted (non-standard JSON extension).
- Input is read by `length` bytes; embedded `\0` is allowed and the buffer does not need to be NUL-terminated.

## Data Structures

```c
enum json_type_t {
    JSON_NONE,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_INTEGER,
    JSON_DOUBLE,
    JSON_STRING,
    JSON_BOOLEAN,
    JSON_NULL,
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
    /* `reserved` is used internally by the parser; do not access it from user code. */
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

- `json_parse` — Parse JSON text of `length` bytes into a tree of `json_value_t` nodes. Returns the root on success, `NULL` on failure.
  `errbuf` receives the error message; it may be `NULL` if not needed, otherwise the caller **must provide at least 256 bytes**.
  Error messages are of the form `line 12: Unexpected ',' in object` (line number only).
- `json_free` — Free the entire parse tree. Implemented iteratively, so deeply nested JSON will not blow the C stack.

## Example

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
