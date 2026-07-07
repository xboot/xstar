# UUID Generator (uuid)

Generate UUID strings conforming to RFC 4122 version 4.

## Format

```
xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
```

- Version number is fixed to `4` (first character of the third segment), indicating random generation
- Variant identifier `y` is one of `8`, `9`, `a`, `b` (first character of the fourth segment), conforming to RFC 4122 variant specification
- Remaining characters are random hexadecimal digits, totaling 36 characters (including 4 hyphens)

Example: `f47ac10b-58cc-4372-a567-0e02b2c3d479`

## API

```c
const char * uuid4(char * buf);
```

Generate and return a UUID v4 string pointer.

- If `buf` is `NULL`, an internal static buffer is used; each call overwrites the previous result
- If `buf` is non-`NULL`, the UUID is written into the user-provided buffer (at least 37 bytes), and the result can be safely retained

> When `buf` is `NULL`, the function is not thread-safe; do not call concurrently from multiple threads.

## Usage Examples

### Generating a Unique Identifier

```c
const char * id = uuid4(NULL);
/* id points to an internal static buffer; will be invalidated on next call */
```

### Retaining the UUID Value

```c
char buf[37];
const char * id = uuid4(buf);
/* buf holds the UUID value and can be safely retained */
```

### Dynamic String Storage

```c
struct ds_t * ds = ds_alloc();
ds_copy(ds, "%s", uuid4(NULL));
/* ds holds the UUID value, unaffected by subsequent uuid4 calls */
ds_free(ds);
```
