# Dynamic Buffer (db)

Dynamic byte buffer for automatic capacity expansion management of binary data. Capacity grows by powers of 2, suitable for scenarios such as protocol parsing and data assembly.

## Data Structure

```c
struct db_t {
    unsigned char * data;   /* Data pointer */
    size_t length;          /* Current data length */
    size_t capacity;        /* Current capacity */
};
```

## API

### Allocation and Deallocation

```c
struct db_t * db_alloc(size_t size);
```

Allocate a buffer, `size` is the pre-allocated capacity (minimum 32, automatically rounded up to a power of 2). Returns NULL on failure.

```c
void db_free(struct db_t * db);
```

Free the buffer and the structure.

### Accessors

```c
size_t db_length(struct db_t * db);     /* Data length */
size_t db_capacity(struct db_t * db);   /* Current capacity */
unsigned char * db_data(struct db_t * db); /* Data pointer */
```

Safe to pass NULL, returns 0 or NULL.

### Operations

```c
void db_clear(struct db_t * db);
```

Clear data (length set to zero, memory not freed).

```c
int db_append(struct db_t * db, const void * buf, size_t len);
int db_append_byte(struct db_t * db, unsigned char byte);
```

Append data to the end. Returns 1 on success, 0 on failure.

```c
int db_prepend(struct db_t * db, const void * buf, size_t len);
int db_prepend_byte(struct db_t * db, unsigned char byte);
```

Insert data at the head (existing data is shifted back). Returns 1 on success, 0 on failure.

```c
const char * db_safe_cstr(struct db_t * db);
```

Temporarily write `\0` at the end of the data, return a read-only pointer that can safely be used as a C string. Does not change length. Returns empty string `""` on failure.

> Note: Subsequent append/prepend operations may invalidate the returned pointer.

## Expansion Strategy

When capacity is insufficient, it grows by 2x until the requirement is met. Initial capacity is automatically rounded up to a power of 2 (minimum 32).

## Usage Example

```c
struct db_t * db = db_alloc(64);

db_append(db, "hello", 5);
db_append_byte(db, ' ');
db_append(db, "world", 5);
db_prepend(db, ">> ", 3);

/* Use as a string */
const char * s = db_safe_cstr(db);  /* ">> hello world" */

db_free(db);
```
