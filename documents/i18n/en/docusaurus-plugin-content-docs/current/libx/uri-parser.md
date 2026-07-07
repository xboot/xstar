# URI Parser (uri)

A URI string parsing module that splits a URI into its constituent components.

## URI Format

```
scheme:[//[user[:pass]@]host[:port]][/path][?query][#fragment]
```

Example:

```
https://admin:secret@example.com:8080/api/data?key=value#section
  │       │     │       │           │    │        │          │
scheme   user   pass   host       port  path    query    fragment
```

## Data Structure

```c
struct uri_t {
    char * scheme;    /* Scheme name, e.g. "http", "ftp" */
    char * user;      /* Username */
    char * pass;      /* Password */
    char * host;      /* Hostname or IP (supports IPv6 [::1] format) */
    int port;         /* Port number, 0 if unspecified */
    char * path;      /* Path */
    char * query;     /* Query string (without '?') */
    char * fragment;  /* Fragment identifier (without '#') */
};
```

All string fields are independently allocated copies; absent parts are NULL. `port` is 0 when unspecified.

## API

```c
struct uri_t * uri_alloc(const char * s);
```

Parse a URI string and return a split `uri_t` structure. Returns NULL on parse failure. Allocates independent memory for each field internally.

```c
void uri_free(struct uri_t * uri);
```

Free the uri_t and all its string fields.

## Parsing Rules

- **scheme** — Must start with a letter, may contain letters, digits, `+`, `-`, `.`, terminated by `:`
- **authority** — The part starting with `//`, containing user info, host, and port
- **user:pass** — The part before `@`, with `:` separating username and password (password may be omitted)
- **host** — Supports IPv6 addresses (enclosed in brackets, e.g., `[::1]`)
- **port** — Numeric only, range 0~65535
- **path** — The part after authority, before `?` or `#`
- **query** — The part after `?`, before `#`
- **fragment** — The part after `#`
- Control characters are not allowed

## Usage Examples

### Parsing a Full URI

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

### Parsing a Partial URI

```c
struct uri_t * uri = uri_alloc("mailto:test@example.com");
/* uri->scheme = "mailto" */
/* uri->path   = "test@example.com" */
/* Remaining fields are NULL or 0 */
uri_free(uri);
```

### Parsing an IPv6 Address

```c
struct uri_t * uri = uri_alloc("http://[::1]:3000/path");
/* uri->host = "[::1]" */
/* uri->port = 3000 */
uri_free(uri);
```
