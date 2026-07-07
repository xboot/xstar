# libcjson

JSON 解析与生成库，基于 [cJSON](https://github.com/DaveGamble/cJSON) 移植，使用 XOS API 替代标准 C 库调用。

## 结构体

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

## 节点类型

| 类型 | 说明 |
|------|------|
| `CJSON_INVALID` | 无效 |
| `CJSON_FALSE` | false |
| `CJSON_TRUE` | true |
| `CJSON_NULL` | null |
| `CJSON_NUMBER` | 数值 |
| `CJSON_STRING` | 字符串 |
| `CJSON_ARRAY` | 数组 |
| `CJSON_OBJECT` | 对象 |
| `CJSON_RAW` | 原始 JSON 字符串 |
| `CJSON_ISREFERENCE` | 引用标记（不拥有数据） |
| `CJSON_STRINGISCONST` | 常量字符串标记 |

类型判断宏：

```c
cjson_is_invalid/item)
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

## 解析

| 函数 | 说明 |
|------|------|
| `cjson_parse(value)` | 解析 JSON 字符串 |
| `cjson_parse_with_length(value, len)` | 解析指定长度的 JSON |
| `cjson_parse_with_opts(value, return_parse_end, require_null_terminated)` | 带选项解析 |
| `cjson_parse_with_length_opts(value, len, return_parse_end, require_null_terminated)` | 带选项和长度解析 |

## 打印

| 函数 | 说明 |
|------|------|
| `cjson_print(item)` | 打印格式化 JSON |
| `cjson_print_unformatted(item)` | 打印紧凑 JSON |
| `cjson_print_buffered(item, prebuffer, fmt)` | 指定预分配缓冲 |
| `cjson_print_preallocated(item, buffer, length, format)` | 写入预分配缓冲 |
| `cjson_minify(json)` | 原地压缩 JSON 字符串 |

## 访问

| 函数 | 说明 |
|------|------|
| `cjson_get_array_size(array)` | 获取数组长度 |
| `cjson_get_array_item(array, index)` | 获取数组元素 |
| `cjson_get_object_item(object, string)` | 获取对象成员（不区分大小写） |
| `cjson_get_object_item_case_sensitive(object, string)` | 获取对象成员（区分大小写） |
| `cjson_has_object_item(object, string)` | 检查成员是否存在 |
| `cjson_get_string_value(item)` | 获取字符串值 |
| `cjson_get_number_value(item)` | 获取数值 |

遍历数组：

```c
struct cjson_t * element;
cjson_array_for_each(element, array) {
    /* 处理 element */
}
```

## 创建

| 函数 | 说明 |
|------|------|
| `cjson_create_null/true/false/bool(value)` | 创建布尔/null |
| `cjson_create_number(num)` | 创建数值 |
| `cjson_create_string(str)` | 创建字符串 |
| `cjson_create_raw(raw)` | 创建原始 JSON |
| `cjson_create_array/object()` | 创建数组/对象 |
| `cjson_create_int/float/double_array(numbers, count)` | 从数组创建 |
| `cjson_create_string_array(strings, count)` | 从字符串数组创建 |

引用创建（不复制数据，只引用指针）：

`cjson_create_string_reference(str)`、`cjson_create_array_reference(child)`、`cjson_create_object_reference(child)`

## 对象添加

| 函数 | 说明 |
|------|------|
| `cjson_add_null/true/false/bool_to_object(obj, name)` | 添加布尔/null |
| `cjson_add_number_to_object(obj, name, number)` | 添加数值 |
| `cjson_add_string_to_object(obj, name, str)` | 添加字符串 |
| `cjson_add_raw_to_object(obj, name, raw)` | 添加原始 JSON |
| `cjson_add_object_to_object(obj, name)` | 添加子对象 |
| `cjson_add_array_to_object(obj, name)` | 添加子数组 |
| `cjson_add_item_to_array(array, item)` | 添加到数组 |
| `cjson_add_item_to_object(obj, name, item)` | 添加到对象 |

## 删除与替换

| 函数 | 说明 |
|------|------|
| `cjson_delete(item)` | 删除并释放整个树 |
| `cjson_detach_item_via_pointer(parent, item)` | 分离节点 |
| `cjson_detach_item_from_array(array, which)` | 从数组分离 |
| `cjson_detach_item_from_object(obj, string)` | 从对象分离 |
| `cjson_delete_item_from_array(array, which)` | 从数组删除 |
| `cjson_delete_item_from_object(obj, string)` | 从对象删除 |
| `cjson_replace_item_in_array(array, which, newitem)` | 替换数组元素 |
| `cjson_replace_item_in_object(obj, string, newitem)` | 替换对象成员 |
| `cjson_duplicate(item, recurse)` | 深拷贝 |
| `cjson_compare(a, b, case_sensitive)` | 比较两个节点 |

## 辅助宏

```c
cjson_set_int_value(object, number)      /* 设置整数值 */
cjson_set_number_value(object, number)    /* 设置数值 */
cjson_set_bool_value(object, boolValue)   /* 设置布尔值 */
cjson_array_for_each(element, array)      /* 遍历数组 */
```

## 配置

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `CJSON_NESTING_LIMIT` | 1000 | 最大嵌套深度 |
| `CJSON_CIRCULAR_LIMIT` | 10000 | 最大递归深度 |
