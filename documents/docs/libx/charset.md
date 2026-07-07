# 字符集转换 (charset)

Unicode 字符集转换，支持 UTF-8、UCS-4、UTF-16 之间的相互转换，以及字符宽度计算。

## API

```c
ssize_t utf8_to_ucs4(uint32_t * dst, size_t dsz, const char * src, size_t ssz, const char ** end);
char * ucs4_to_utf8(uint32_t * src, size_t ssz, char * dst, size_t dsz);
ssize_t utf8_to_utf16(uint16_t * dst, size_t dsz, const char * src, size_t ssz, const char ** end);
char * utf16_to_utf8(char * dst, uint16_t * src, size_t size);
ssize_t utf8_to_ucs4_alloc(const char * src, uint32_t ** dst, uint32_t ** pos);
char * ucs4_to_utf8_alloc(uint32_t * src, size_t size);
int ucs4_width(uint32_t uc);
size_t utf8_width(const char * s);
const char * utf8_to_code(const char * p, uint32_t * code);
int utf8_is_valid(const char * s, size_t size);
size_t utf8_strlen(const char * s);
```

- `utf8_to_ucs4` — UTF-8 转 UCS-4，`end` 返回停止位置
- `ucs4_to_utf8` — UCS-4 转 UTF-8，返回写入指针
- `utf8_to_utf16` — UTF-8 转 UTF-16
- `utf16_to_utf8` — UTF-16 转 UTF-8
- `utf8_to_ucs4_alloc` — UTF-8 转 UCS-4（自动分配），`pos` 返回停止位置
- `ucs4_to_utf8_alloc` — UCS-4 转 UTF-8（自动分配）
- `ucs4_width` — 单个 UCS-4 字符显示宽度（中文为 2，ASCII 为 1）
- `utf8_width` — UTF-8 字符串显示宽度
- `utf8_to_code` — 解码单个码点，返回下一个字符指针
- `utf8_is_valid` — 验证 UTF-8 编码是否合法
- `utf8_strlen` — UTF-8 字符串码点数

## 使用示例

### UTF-8 转 UCS-4

```c
const char * text = "你好";
uint32_t * ucs4;
uint32_t * pos;
ssize_t len = utf8_to_ucs4_alloc(text, &ucs4, &pos);
xos_mem_free(ucs4);
```

### 计算字符串显示宽度

```c
const char * text = "Hello世界";
size_t w = utf8_width(text);
```

### 遍历 UTF-8 码点

```c
const char * p = "你好";
uint32_t code;
while(*p)
{
    p = utf8_to_code(p, &code);
}
```
