# File System (xfs)

XSTAR's virtual file system that unifies access to different storage media through pluggable archiver backends, supporting CPIO archives, real directories, and the KOBJ virtual file system, with standard file operations including mounting, file read/write, and directory traversal.

## Data Structures

### File System Context

```c
struct xfs_context_t {
    struct xfs_path_t mounts;   /* Mount point list head */
    struct mutex_t lock;        /* Mutex protecting mount point operations */
};
```

### Mount Point

```c
struct xfs_path_t {
    char * mpath;                       /* Mount path */
    void * mhandle;                     /* Archiver mount handle */
    int writable;                       /* Whether writable */
    struct xfs_archiver_t * archiver;   /* Archiver instance */
    struct list_head_t list;            /* List node */
};
```

### File Handle

```c
struct xfs_file_t {
    struct xfs_context_t * ctx;   /* Owning context */
    struct xfs_path_t * path;     /* Owning mount point */
    void * fhandle;               /* File handle */
};
```

## How It Works

### Archiver Architecture

XFS abstracts different storage backends through the archiver interface (`struct xfs_archiver_t`). Each archiver implements a unified set of operations (mount/umount/walk/open/read/write etc.). Archivers are registered to a global list via `register_archiver()`. `mount_archiver()` tries each archiver in registration order; the first one that successfully mounts takes over the path.

### Storage Backends

| Archiver | Description | Writable |
|----------|-------------|----------|
| cpio | CPIO newc format archive, read from block devices, used for romdisk | No |
| dir | Host filesystem directory, accessed via XOS file interface | Yes |
| sys | KOBJ virtual file system, mounted at `"sys"`, accesses kernel objects | Yes |

### Auto-Mount

`xfs_alloc()` automatically mounts three paths when creating a context:

1. `"blk-romdisk.0"` — read-only, handled by the cpio archiver, accesses the compile-time embedded romdisk
2. `"sys"` — read-write, handled by the sys archiver, accesses the KOBJ virtual file system
3. Current working directory — read-write, handled by the dir archiver, accesses the host filesystem

### Lookup Order

File lookup traverses mount points in reverse order (later mounts take priority):

- **Read operations** (open_read/isdir/isfile/walk): searches from the last mount point backward, returns the first match
- **Write operations** (open_write/open_append/mkdir/remove): searches from the last writable mount point backward

### Path Normalization

All paths are processed through `normal_path()` before operations:

- Strips leading `/`
- Rejects `.` and `..` path components (security restriction, prevents path traversal)
- Rejects `:` and `\\` characters

## API

### Context Management

| Function | Description |
|----------|-------------|
| `xfs_alloc()` | Create a file system context, auto-mounts romdisk, sys, and current working directory |
| `xfs_free(ctx)` | Free the context, unmounts all mount points |

### Mount Management

| Function | Description |
|----------|-------------|
| `xfs_mount(ctx, path, writable)` | Mount a path, `writable` requests write permission |
| `xfs_umount(ctx, path)` | Unmount a path |

### Directory Operations

| Function | Description |
|----------|-------------|
| `xfs_walk(ctx, name, cb, data)` | Traverse a directory, calling `cb(dir, name, data)` for each entry |
| `xfs_isdir(ctx, name)` | Check if the path is a directory |
| `xfs_isfile(ctx, name)` | Check if the path is a file |
| `xfs_mode(ctx, name)` | Get the file mode |
| `xfs_mkdir(ctx, name)` | Create a directory (writable mount points only) |
| `xfs_remove(ctx, name)` | Remove a file or directory (writable mount points only) |

### File Operations

| Function | Description |
|----------|-------------|
| `xfs_open_read(ctx, name)` | Open a file for reading |
| `xfs_open_write(ctx, name)` | Open a file for writing (writable mount points only) |
| `xfs_open_append(ctx, name)` | Open a file for appending (writable mount points only) |
| `xfs_read(file, buf, size)` | Read file content, returns actual bytes read |
| `xfs_write(file, buf, size)` | Write file content, returns actual bytes written |
| `xfs_seek(file, offset)` | Set the file read/write position |
| `xfs_tell(file)` | Get the current read/write position |
| `xfs_length(file)` | Get the file length |
| `xfs_flush(file)` | Flush the file buffer |
| `xfs_close(file)` | Close the file and release resources |

## Usage Examples

### Reading a File

```c
#include <kernel/xfs/xfs.h>

struct xfs_context_t * ctx = xfs_alloc();
struct xfs_file_t * file = xfs_open_read(ctx, "dtree/default.json");
if(file)
{
    int64_t len = xfs_length(file);
    char * buf = xos_mem_malloc(len + 1);
    if(buf)
    {
        xfs_read(file, buf, len);
        buf[len] = '\0';
        LOG("content: %s\n", buf);
        xos_mem_free(buf);
    }
    xfs_close(file);
}
xfs_free(ctx);
```

### Writing a File

```c
struct xfs_context_t * ctx = xfs_alloc();
struct xfs_file_t * file = xfs_open_write(ctx, "setting.db");
if(file)
{
    const char * data = "hello world";
    xfs_write(file, (void *)data, strlen(data));
    xfs_flush(file);
    xfs_close(file);
}
xfs_free(ctx);
```

### Traversing a Directory

```c
static void list_cb(const char * dir, const char * name, void * data)
{
    shell_printf("%s/%s\r\n", dir, name);
}

void list_files(void)
{
    struct xfs_context_t * ctx = xfs_alloc();
    xfs_walk(ctx, "", list_cb, NULL);
    xfs_free(ctx);
}
```

### Accessing the KOBJ Virtual File System

```c
struct xfs_context_t * ctx = xfs_alloc();
struct xfs_file_t * file = xfs_open_read(ctx, "sys/clk/clk-fixed.0/summary");
if(file)
{
    char buf[256];
    int64_t len = xfs_read(file, buf, sizeof(buf) - 1);
    if(len > 0)
    {
        buf[len] = '\0';
        shell_printf("%s\r\n", buf);
    }
    xfs_close(file);
}
xfs_free(ctx);
```

## Notes

- Each module that needs file access should create its own context via `xfs_alloc()` and call `xfs_free()` when done
- The romdisk (`blk-romdisk.0`) is read-only; write operations automatically skip this mount point
- The path separator is `/`; `.` and `..` path components are not supported
- Archivers are initialized early during system startup via `pure_initcall()`
- `mount_archiver()` tries mounting in registration order: cpio before dir before sys
- The KOBJ virtual file system (sys) provides read/write access to system kernel objects (devices, drivers, etc.)
