# libcjson

JSON parsing and generation library, ported from [cJSON](https://github.com/DaveGamble/cJSON), using XOS API instead of standard C library calls.

## Struct

```c
struct cjson_t {
    struct cjson_t * next;
    struct cjson_t * prev;
    struct cjson_t * child;
    int type;
    char * valuestring;
    int valueint;
    double valuedouble;
    char * string;
};
```

## Node Types

| Type | Description |
|------|-------------|
| `CJSON_INVALID` | Invalid |
| `CJSON_FALSE` | false |
| `CJSON_TRUE` | true |
| `CJSON_NULL` | null |
| `CJSON_NUMBER` | Number |
| `CJSON_STRING` | String |
| `CJSON_ARRAY` | Array |
| `CJSON_OBJECT` | Object |
| `CJSON_RAW` | Raw JSON string |
| `CJSON_ISREFERENCE` | Reference marker (does not own data) |
| `CJSON_STRINGISCONST` | Constant string marker |

Type check macros:

```c
cjson_is_invalid(item)
cjson_is_false(item)
cjson_is_true(item)
cjson_is_bool(item)
cjson_is_null(item)
cjson_is_number(item)
cjson_is_string(item)
cjson_is_array(item)
cjson_is_object(item)
cjson_is_raw(item)
```

## Parsing

| Function | Description |
|----------|-------------|
| `cjson_parse(value)` | Parse a JSON string |
| `cjson_parse_with_length(value, len)` | Parse JSON with specified length |
| `cjson_parse_with_opts(value, return_parse_end, require_null_terminated)` | Parse with options |
| `cjson_parse_with_length_opts(value, len, return_parse_end, require_null_terminated)` | Parse with options and length |

## Printing

| Function | Description |
|----------|-------------|
| `cjson_print(item)` | Print formatted JSON |
| `cjson_print_unformatted(item)` | Print compact JSON |
| `cjson_print_buffered(item, prebuffer, fmt)` | Print with pre-allocated buffer |
| `cjson_print_preallocated(item, buffer, length, format)` | Write into pre-allocated buffer |
| `cjson_minify(json)` | In-place minify JSON string |

## Access

| Function | Description |
|----------|-------------|
| `cjson_get_array_size(array)` | Get array length |
| `cjson_get_array_item(array, index)` | Get array element |
| `cjson_get_object_item(object, string)` | Get object member (case-insensitive) |
| `cjson_get_object_item_case_sensitive(object, string)` | Get object member (case-sensitive) |
| `cjson_has_object_item(object, string)` | Check if member exists |
| `cjson_get_string_value(item)` | Get string value |
| `cjson_get_number_value(item)` | Get number value |

Iterate array:

```c
struct cjson_t * element;
cjson_array_for_each(element, array) {
    /* process element */
}
```

## Creation

| Function | Description |
|----------|-------------|
| `cjson_create_null/true/false/bool(value)` | Create boolean/null |
| `cjson_create_number(num)` | Create number |
| `cjson_create_string(str)` | Create string |
| `cjson_create_raw(raw)` | Create raw JSON |
| `cjson_create_array/object()` | Create array/object |
| `cjson_create_int/float/double_array(numbers, count)` | Create from array |
| `cjson_create_string_array(strings, count)` | Create from string array |

Reference creation (does not copy data, only references pointer):

`cjson_create_string_reference(str)`, `cjson_create_array_reference(child)`, `cjson_create_object_reference(child)`

## Object Add

| Function | Description |
|----------|-------------|
| `cjson_add_null/true/false/bool_to_object(obj, name)` | Add boolean/null |
| `cjson_add_number_to_object(obj, name, number)` | Add number |
| `cjson_add_string_to_object(obj, name, str)` | Add string |
| `cjson_add_raw_to_object(obj, name, raw)` | Add raw JSON |
| `cjson_add_object_to_object(obj, name)` | Add child object |
| `cjson_add_array_to_object(obj, name)` | Add child array |
| `cjson_add_item_to_array(array, item)` | Add to array |
| `cjson_add_item_to_object(obj, name, item)` | Add to object |

## Delete and Replace

| Function | Description |
|----------|-------------|
| `cjson_delete(item)` | Delete and free entire tree |
| `cjson_detach_item_via_pointer(parent, item)` | Detach node |
| `cjson_detach_item_from_array(array, which)` | Detach from array |
| `cjson_detach_item_from_object(obj, string)` | Detach from object |
| `cjson_delete_item_from_array(array, which)` | Delete from array |
| `cjson_delete_item_from_object(obj, string)` | Delete from object |
| `cjson_replace_item_in_array(array, which, newitem)` | Replace array element |
| `cjson_replace_item_in_object(obj, string, newitem)` | Replace object member |
| `cjson_duplicate(item, recurse)` | Deep copy |
| `cjson_compare(a, b, case_sensitive)` | Compare two nodes |

## Helper Macros

```c
cjson_set_int_value(object, number)      /* Set integer value */
cjson_set_number_value(object, number)    /* Set number value */
cjson_set_bool_value(object, boolValue)   /* Set boolean value */
cjson_array_for_each(element, array)      /* Iterate array */
```

## Configuration

| Option | Default | Description |
|--------|---------|-------------|
| `CJSON_NESTING_LIMIT` | 1000 | Maximum nesting depth |
| `CJSON_CIRCULAR_LIMIT` | 10000 | Maximum recursion depth |
