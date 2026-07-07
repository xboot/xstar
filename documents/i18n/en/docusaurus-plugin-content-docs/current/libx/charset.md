# Character Set Conversion (charset)

Unicode character set conversion supporting UTF-8, UCS-4, UTF-16 inter-conversion and character width calculation.

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

- `utf8_to_ucs4` — UTF-8 to UCS-4, `end` returns stop position
- `ucs4_to_utf8` — UCS-4 to UTF-8, returns write pointer
- `utf8_to_utf16` — UTF-8 to UTF-16
- `utf16_to_utf8` — UTF-16 to UTF-8
- `utf8_to_ucs4_alloc` — UTF-8 to UCS-4 (auto-allocated), `pos` returns stop position
- `ucs4_to_utf8_alloc` — UCS-4 to UTF-8 (auto-allocated)
- `ucs4_width` — Display width of a single UCS-4 character (2 for CJK, 1 for ASCII)
- `utf8_width` — Display width of a UTF-8 string
- `utf8_to_code` — Decode a single code point, returns pointer to next character
- `utf8_is_valid` — Validate UTF-8 encoding
- `utf8_strlen` — Number of code points in a UTF-8 string

## Example

### UTF-8 to UCS-4

```c
const char * text = "Hello";
uint32_t * ucs4;
uint32_t * pos;
ssize_t len = utf8_to_ucs4_alloc(text, &ucs4, &pos);
xos_mem_free(ucs4);
```

### Calculate String Display Width

```c
const char * text = "HelloWorld";
size_t w = utf8_width(text);
```

### Iterate UTF-8 Code Points

```c
const char * p = "Hello";
uint32_t code;
while(*p)
{
    p = utf8_to_code(p, &code);
}
```
