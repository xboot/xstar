# Dynamic String (ds)

Dynamic string library providing auto-expanding string operations including append, insert, delete, find, replace, split, and more.

## Data Structure

```c
struct ds_t {
    char * data;       /* String data terminated by '\0' */
    size_t length;     /* String length (excluding '\0') */
    size_t capacity;   /* Current capacity */
};
```

## API

### Allocation and Deallocation

```c
struct ds_t * ds_alloc(void);
```

Allocate an empty string, initial capacity 16.

```c
struct ds_t * ds_alloc_from_str(const char * str);
```

Create from a C string, copying the content.

```c
struct ds_t * ds_alloc_from_buf(const void * buf, int len);
```

Create from the specified buffer, copying `len` bytes and automatically appending a `'\0'` terminator. Returns an empty string when `buf` is NULL or `len <= 0`.

```c
void ds_free(struct ds_t * ds);
```

Free the string and the structure.

### Accessors

```c
size_t ds_length(struct ds_t * ds);    /* String length */
size_t ds_capacity(struct ds_t * ds);  /* Current capacity */
int ds_isempty(struct ds_t * ds);      /* Returns 1 if empty */
const char * ds_cstr(struct ds_t * ds); /* C string pointer, NULL-safe, returns "" */
```

### Copy and Substring

```c
struct ds_t * ds_clone(struct ds_t * ds);
```

Deep copy, returns a new ds_t.

```c
struct ds_t * ds_substr(struct ds_t * ds, size_t start, size_t len);
```

Extract a substring, returns a new ds_t. Returns an empty string when `start` exceeds length, `len` is automatically truncated when out of range.

### Clear and Trim

```c
void ds_clear(struct ds_t * ds);
```

Clear the string (length set to zero, memory not freed).

```c
void ds_trim(struct ds_t * ds);
```

Remove leading and trailing whitespace characters.

### Append and Prepend

```c
int ds_append(struct ds_t * ds, const char * str);
int ds_append_char(struct ds_t * ds, const char c);
int ds_prepend(struct ds_t * ds, const char * str);
```

Returns 1 on success, 0 on failure.

### Insert and Delete

```c
int ds_insert(struct ds_t * ds, size_t pos, const char * str);
```

Insert a string at position `pos`, appended to the end when `pos` exceeds length.

```c
int ds_delete(struct ds_t * ds, size_t start, size_t len);
```

Delete `len` characters starting from position `start`, automatically truncated when out of range.

### Formatting

```c
int ds_copy(struct ds_t * ds, const char * fmt, ...);
```

**Replace** the entire string content using printf-style formatting. Returns the number of characters written, 0 on failure.

```c
int ds_concat(struct ds_t * ds, const char * fmt, ...);
```

**Append** to the end using printf-style formatting. Returns the number of characters appended, 0 on failure.

### Find

```c
int ds_find(struct ds_t * ds, const char * sub);
```

Find a substring from the left, returns the position index, -1 if not found. Returns length for empty substring.

```c
int ds_rfind(struct ds_t * ds, const char * sub);
```

Find the last occurrence of a substring from the right, -1 if not found. Returns length for empty substring.

### Replace

```c
int ds_replace(struct ds_t * ds, const char * ostr, const char * nstr);
```

Replace all matching substrings, returns the number of replacements.

```c
int ds_replace_first(struct ds_t * ds, const char * ostr, const char * nstr);
int ds_replace_last(struct ds_t * ds, const char * ostr, const char * nstr);
```

Replace the first/last match respectively. Returns 1 on success, 0 if not found.

### Split Iteration

```c
struct ds_iter_t * ds_iter_alloc(struct ds_t * ds, const char * delim);
void ds_iter_free(struct ds_iter_t * iter);
char * ds_iter_next(struct ds_iter_t * iter);
```

Split a string by delimiter, iterate to get each token. `ds_iter_next` returns NULL when finished.

> Note: The iterator internally copies the original string data; modifying the original ds_t does not affect the iteration. The returned token pointer becomes invalid after `ds_iter_free`.

## Usage Examples

### Basic Operations

```c
struct ds_t * s = ds_alloc_from_str("hello");
ds_append(s, " world");
ds_prepend(s, ">> ");
/* ds_cstr(s) == ">> hello world" */
ds_free(s);
```

### Formatting

```c
struct ds_t * s = ds_alloc();
ds_copy(s, "count: %d", 42);
ds_concat(s, ", name: %s", "test");
/* ds_cstr(s) == "count: 42, name: test" */
ds_free(s);
```

### Find and Replace

```c
struct ds_t * s = ds_alloc_from_str("aabbccaabb");
ds_replace(s, "aa", "XX");        /* "XXbbccXXbb", returns 2 */
ds_replace_first(s, "bb", "YY");  /* "XXYYccXXbb", returns 1 */
ds_free(s);
```

### Split Iteration

```c
struct ds_t * s = ds_alloc_from_str("one,two,three");
struct ds_iter_t * iter = ds_iter_alloc(s, ",");
char * token;
while((token = ds_iter_next(iter)) != NULL)
{
    /* in order: "one", "two", "three" */
}
ds_iter_free(iter);
ds_free(s);
```
