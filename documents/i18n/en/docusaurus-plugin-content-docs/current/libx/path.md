# Path Utility (path)

Filesystem path manipulation utilities: classify absolute/relative paths, extract basename, dirname, and file extension.

## API

```c
int is_absolute_path(const char * path);
int is_relative_path(const char * path);
char * path_basename(char * path);
char * path_dirname(char * path);
const char * path_fileext(const char * filename);
```

- `is_absolute_path` — Return 1 if path is absolute (`"/"`-prefixed or Windows drive-letter path like `"C:\"`), 0 otherwise
- `is_relative_path` — Return 1 if path is relative (not absolute), 0 otherwise
- `path_basename` — Return pointer to the basename component within `path` (after the last '/')
- `path_dirname` — Return pointer to the dirname component within `path` (modifies path in place by inserting '\0' after dirname)
- `path_fileext` — Return pointer to the file extension within `filename` (after the last '.'), or empty string if no extension

## Example

```c
is_absolute_path("/usr/bin");     /* 1 */
is_absolute_path("C:\\Windows");  /* 1 */
is_relative_path("src/main.c");   /* 1 */

char buf1[] = "/usr/bin/gcc";
char * base = path_basename(buf1);  /* "gcc" */

char buf2[] = "/usr/bin/gcc";
char * dir = path_dirname(buf2);    /* "/usr/bin" */

const char * ext = path_fileext("main.c");  /* "c" */
```
