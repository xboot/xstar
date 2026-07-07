# Persistence (setting)

A key-value persistent storage module, providing system configuration read/write, deferred writing, and automatic persistence, with support for both block device and file storage backends, and CRC32 integrity verification.

## Data Structure

```c
struct setting_t {
    struct timer_t timer;       /* Deferred write timer */
    struct hmap_t * map;        /* Key-value hash map */
    struct mutex_t lock;        /* Mutex protecting concurrent access */
    int dirty;                  /* Dirty flag, 1 means unwritten modifications */

    uint64_t (*capacity)(struct setting_t *);              /* Get storage capacity */
    uint64_t (*read)(struct setting_t *, void *, uint64_t, uint64_t);   /* Read data */
    uint64_t (*write)(struct setting_t *, void *, uint64_t, uint64_t);  /* Write data */
    void (*sync)(struct setting_t *);                      /* Flush to media */
    void * priv;                /* Backend private data */
};
```

## How It Works

### Storage Backend

At initialization, the storage backend is automatically detected by priority:

1. **Block Device Backend**: Searches registered block devices for one whose name contains `"setting"`, uses `block_read`/`block_write`/`block_sync` for direct read/write
2. **File Backend**: Creates/opens a `setting.db` file in the XFS file system, with a fixed capacity of 1MB

If neither is available, persistence is disabled, but in-memory key-value pairs can still be read and written normally.

### File Format

Persistent data uses a custom binary format:

```
Offset  Size    Content
0       4       Magic "STAR"
4       4       CRC32 checksum (covers offset 8 to end)
8       4       Data length (little-endian, in bytes)
12      N       Key-value data, format: key:value|key:value|...
```

Key-value pairs are written in sorted hash map order. The CRC32 covers the length field and data content. If verification fails on load, the data is discarded.

### Deferred Writing

Modification operations (`setting_set`/`setting_clear`) do not write to storage immediately. Instead, they set the dirty flag and start a 10-second timer. When the timer fires, `setting_timer_function` serializes all non-`volatile.` prefixed key-value pairs and writes them to storage.

### Volatile Keys

Keys prefixed with `volatile.` are not persisted to storage media; they exist only in memory. Suitable for temporary configuration and runtime state.

### Forced Sync

`setting_sync()` advances the timer to fire in 1ms and blocks until the dirty flag is cleared, ensuring data has been written to storage.

## API

| Function | Description |
|----------|-------------|
| `setting_set(key, value)` | Set a key-value pair; `value` of NULL deletes the key; non-`volatile.` keys trigger deferred write |
| `setting_get(key, def)` | Get a value by key; returns `def` if the key does not exist |
| `setting_clear()` | Clear all key-value pairs, triggers deferred write |
| `setting_sync()` | Force all modifications to be written to storage immediately, blocks until complete |
| `setting_foreach(cb)` | Iterate all key-value pairs in sorted key order, calling the callback for each pair |
| `do_init_setting()` | Initialize the persistence module, detect backend and load existing data (automatically called at system startup) |

## Usage Examples

### Read/Write Configuration

```c
#include <kernel/core/setting.h>

/* Write configuration */
setting_set("wifi.ssid", "MyNetwork");
setting_set("wifi.password", "12345678");

/* Read configuration */
const char * ssid = setting_get("wifi.ssid", "default");
const char * pwd = setting_get("wifi.password", NULL);

/* Delete configuration */
setting_set("wifi.ssid", NULL);
```

### Volatile Keys

```c
/* Keys with volatile. prefix are not persisted, lost on reboot */
setting_set("volatile.session_id", "abc123");
const char * sid = setting_get("volatile.session_id", NULL);
```

### Iterate All Settings

```c
static void print_setting(const char * key, const char * value)
{
    shell_printf("%s = %s\r\n", key, value);
}

void list_settings(void)
{
    setting_foreach(print_setting);
}
```

### Forced Sync

```c
/* Persist immediately after modifying critical configuration */
setting_set("system.boot_count", "42");
setting_sync();  /* Blocks until write is complete */
```

## Notes

- All operations are protected by a mutex, making them thread-safe
- The deferred write interval is 10 seconds, avoiding frequent writes that wear out storage media
- Keys with the `volatile.` prefix are not persisted; serialization stops at the first `volatile.` key encountered
- The file format uses CRC32 verification; if verification fails on load, all data is ignored
- `do_init_setting()` is automatically called in `xstar_init()`; no manual invocation is needed
- `setting_sync()` internally uses `xos_thread_sleep()` for polling, so it must not be called in interrupt context
