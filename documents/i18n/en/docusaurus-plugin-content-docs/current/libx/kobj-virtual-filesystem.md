# KOBJ Virtual Filesystem (kobj)

This document details XSTAR's KOBJ (Kernel Object) virtual filesystem, including data structures, API interfaces, and usage examples.

## Table of Contents

- [Introduction](#introduction)
- [KOBJ Node Types](#kobj-node-types)
- [KOBJ Structure](#kobj-structure)
- [KOBJ API](#kobj-api)
- [Usage Examples](#usage-examples)
- [Practical Applications](#practical-applications)
- [API Reference](#api-reference)

## Introduction

### Purpose of KOBJ

KOBJ (Kernel Object) is a kernel object whose hierarchical structure is represented as an inverted tree, used to manage various devices, drivers, runtime state, etc.

In the filesystem, kobj nodes are bound through the sysfs interface to enable read and write operations on the node. The sysfs interface has two basic purposes:

1. **External component operation interface**: Applications can operate devices through this interface
2. **Debugging driver modules or viewing runtime state**: Obtain device information by reading/writing kobj nodes

### Core Value

Implementing kobj allows a high level of abstraction for any form of component and serves as the foundation for various components. The system provides a set of APIs to facilitate kobj management.

Understanding kobj is extremely helpful for understanding various core components in XSTAR (such as device drivers, device management, etc.). Careful reading is recommended.

## KOBJ Node Types

kobj has two basic types:

### Directory Type (KOBJ_TYPE_DIR)

- Can attach child nodes
- Similar to directories in a filesystem
- Does not provide read/write interfaces
- Used to organize node hierarchy

### File Type (KOBJ_TYPE_REG)

- Represents leaf nodes of the tree
- Cannot have child nodes
- Provides read or write interfaces
- Used to store and pass data

**Enum definition:**
```c
enum kobj_type_t {
    KOBJ_TYPE_DIR,
    KOBJ_TYPE_REG,
};
```

## KOBJ Structure

### kobj_t Structure Definition

The kobj structure has a name, node type, parent node pointer, current node list entry, child node list head, read/write interface functions, and a private data pointer.

```c
struct kobj_t {
    char * name;                           /* kobj name */
    enum kobj_type_t type;                  /* kobj type DIR or REG */
    struct kobj_t * parent;                /* kobj parent node */
    struct list_head_t entry;               /* kobj list entry */
    struct list_head_t children;            /* kobj child node list */
    ssize_t (*read)(struct kobj_t * kobj, void * buf, size_t size);   /* read interface */
    ssize_t (*write)(struct kobj_t * kobj, void * buf, size_t size);  /* write interface */
    void * priv;                           /* private data */
};
```

### Field Description

| Field | Type | Description |
|-------|------|-------------|
| `name` | `char *` | Node name, must be unique (under the same parent) |
| `type` | `enum kobj_type_t` | Node type: directory or file |
| `parent` | `struct kobj_t *` | Parent node pointer |
| `entry` | `struct list_head_t` | List entry for attaching to parent's children list |
| `children` | `struct list_head_t` | Child node list head |
| `read` | function pointer | Read operation callback (valid only for file type) |
| `write` | function pointer | Write operation callback (valid only for file type) |
| `priv` | `void *` | Private data, can store arbitrary user data |

### Callback Function Types

```c
typedef ssize_t (*kobj_read_t)(struct kobj_t * kobj, void * buf, size_t size);
typedef ssize_t (*kobj_write_t)(struct kobj_t * kobj, void * buf, size_t size);
```

**Callback function parameters:**
- `kobj`: kobj node pointer
- `buf`: Buffer pointer
- `size`: Buffer size

**Callback function return values:**
- Success: Returns the actual number of bytes read/written
- Failure: Returns a negative value

## KOBJ API

### Get Root Node

Get the kobj root node, automatically created if it does not exist.

**Function prototype:**
```c
struct kobj_t * kobj_get_root(void);
```

**Return value:**
- Success: Returns root node pointer
- Failure: Returns NULL

**Description:**
- Root node name is "kobj"
- Root node is a global static variable
- Root node is also the top-level directory node of sysfs
- It is mounted to the `/sys` directory when the filesystem is mounted

### Search Node

Search for a child node with the given name under the parent node.

**Function prototype:**
```c
struct kobj_t * kobj_search(struct kobj_t * parent, const char * name);
```

**Parameters:**
- `parent`: Parent node pointer
- `name`: Name of the node to search for

**Return value:**
- Found: Returns node pointer
- Not found: Returns NULL

**Usage example:**
```c
struct kobj_t *root = kobj_get_root();
struct kobj_t *device = kobj_search(root, "device");
```

### Search or Create Directory

Search for a child node with the given name under the parent node; if it does not exist, create a directory node.

**Function prototype:**
```c
struct kobj_t * kobj_search_directory_with_create(struct kobj_t * parent, const char * name);
```

**Parameters:**
- `parent`: Parent node pointer
- `name`: Directory name

**Return value:**
- Success: Returns directory node pointer
- Failure: Returns NULL

**Description:**
- If the node does not exist, automatically creates a directory node
- If the node exists but is not a directory type, returns NULL
- Commonly used to ensure directory nodes along a path exist

**Usage example:**
```c
struct kobj_t *root = kobj_get_root();
struct kobj_t *device = kobj_search_directory_with_create(root, "device");
struct kobj_t *gpio = kobj_search_directory_with_create(device, "gpio");
```

### Allocate Directory Node

Allocate a kobj node of directory type.

**Function prototype:**
```c
struct kobj_t * kobj_alloc_directory(const char * name);
```

**Parameters:**
- `name`: Directory name

**Return value:**
- Success: Returns the allocated directory node pointer
- Failure: Returns NULL

**Description:**
- Only allocates memory and initializes, does not add to parent
- Requires calling `kobj_add` to add to parent
- Directory nodes do not provide read/write interfaces

**Usage example:**
```c
struct kobj_t *dir = kobj_alloc_directory("my-directory");
if(dir) {
    kobj_add(parent, dir);
}
```

### Allocate File Node

Allocate a kobj node of file type.

**Function prototype:**
```c
struct kobj_t * kobj_alloc_regular(const char * name, kobj_read_t read, kobj_write_t write, void * priv);
```

**Parameters:**
- `name`: File name
- `read`: Read callback function (can be NULL)
- `write`: Write callback function (can be NULL)
- `priv`: Private data pointer (can be NULL)

**Return value:**
- Success: Returns the allocated file node pointer
- Failure: Returns NULL

**Description:**
- Only allocates memory and initializes, does not add to parent
- Requires calling `kobj_add` to add to parent
- File nodes must provide at least one callback function

**Usage example:**
```c
static ssize_t my_read(struct kobj_t * kobj, void * buf, size_t size)
{
    return xos_snprintf(buf, size, "Hello, KOBJ!\n");
}

struct kobj_t *file = kobj_alloc_regular("my-file", my_read, NULL, NULL);
if(file) {
    kobj_add(parent, file);
}
```

### Add Node

Add a node (directory or file) to a parent directory node.

**Function prototype:**
```c
int kobj_add(struct kobj_t * parent, struct kobj_t * kobj);
```

**Parameters:**
- `parent`: Parent node pointer (must be directory type)
- `kobj`: Node pointer to add

**Return value:**
- Success: Returns TRUE
- Failure: Returns FALSE

**Description:**
- Parent node must be directory type
- No duplicate names are allowed under the same parent
- Automatically sets the parent node pointer

### Remove Node

Remove a child node from a parent directory node.

**Function prototype:**
```c
int kobj_remove(struct kobj_t * parent, struct kobj_t * kobj);
```

**Parameters:**
- `parent`: Parent node pointer (must be directory type)
- `kobj`: Node pointer to remove

**Return value:**
- Success: Returns TRUE
- Failure: Returns FALSE

**Description:**
- Only removes from the parent node, does not free memory
- Requires manually calling `kobj_free` to free memory

**Usage example:**
```c
if(kobj_remove(parent, kobj)) {
    kobj_free(kobj);
}
```

### Add Directory Node

Quickly create and add a directory node to a parent node.

**Function prototype:**
```c
int kobj_add_directory(struct kobj_t * parent, const char * name);
```

**Parameters:**
- `parent`: Parent node pointer (must be directory type)
- `name`: Directory name

**Return value:**
- Success: Returns TRUE
- Failure: Returns FALSE

**Description:**
- Automatically allocates a directory node and adds it to the parent
- No duplicate directory names are allowed under the same parent
- If adding fails, automatically frees the allocated memory

**Usage example:**
```c
struct kobj_t *root = kobj_get_root();
kobj_add_directory(root, "device");
kobj_add_directory(root, "class");
```

### Add File Node

Quickly create and add a file node to a parent node.

**Function prototype:**
```c
int kobj_add_regular(struct kobj_t * parent, const char * name, kobj_read_t read, kobj_write_t write, void * priv);
```

**Parameters:**
- `parent`: Parent node pointer (must be directory type)
- `name`: File name
- `read`: Read callback function (can be NULL)
- `write`: Write callback function (can be NULL)
- `priv`: Private data pointer (can be NULL)

**Return value:**
- Success: Returns TRUE
- Failure: Returns FALSE

**Description:**
- Automatically allocates a file node and adds it to the parent
- No duplicate file names are allowed under the same parent
- If adding fails, automatically frees the allocated memory

**Usage example:**
```c
struct kobj_t *root = kobj_get_root();
kobj_add_regular(root, "version", my_read, NULL, NULL);
```

### Free Node

Free the memory occupied by a kobj node.

**Function prototype:**
```c
int kobj_free(struct kobj_t * kobj);
```

**Parameters:**
- `kobj`: Node pointer to free

**Return value:**
- Success: Returns TRUE
- Failure: Returns FALSE

**Description:**
- Frees the node name and node structure
- Does not remove the node from the parent
- Does not recursively free child nodes
- Returns FALSE if the directory node still has children (refuses to free)

**Usage example:**
```c
if(kobj_remove(parent, kobj)) {
    kobj_free(kobj);
}
```

### Recursively Remove Node

Recursively remove a node and all its child nodes.

**Function prototype:**
```c
int kobj_remove_self(struct kobj_t * kobj);
```

**Parameters:**
- `kobj`: Node pointer to remove

**Return value:**
- Success: Returns TRUE
- Failure: Returns FALSE

**Description:**
- If the node is a directory type, first recursively removes all child nodes
- Removes the node from the parent
- Frees the node memory
- Commonly used to clean up an entire subtree

**Usage example:**
```c
struct kobj_t *my_dir = kobj_search(root, "my-directory");
if(my_dir) {
    kobj_remove_self(my_dir);
}
```

## Usage Examples

### Example 1: Create a simple kobj tree

```c
#include <xstar.h>
#include <libx/kobj.h>

static void create_simple_kobj_tree(void)
{
    struct kobj_t *root, *device, *gpio, *file;

    /* Get root node */
    root = kobj_get_root();

    /* Create device directory */
    device = kobj_search_directory_with_create(root, "device");

    /* Create device/gpio subdirectory */
    gpio = kobj_search_directory_with_create(device, "gpio");

    /* Create file node */
    file = kobj_alloc_regular("test-file", NULL, NULL, NULL);
    if(file) {
        kobj_add(gpio, file);
    }
}
```

### Example 2: Provide read/write interfaces

```c
static int counter = 0;

static ssize_t counter_read(struct kobj_t * kobj, void * buf, size_t size)
{
    return xos_snprintf(buf, size, "%d\n", counter);
}

static ssize_t counter_write(struct kobj_t * kobj, void * buf, size_t size)
{
    char str[32];
    int len = (size < sizeof(str) - 1) ? size : sizeof(str) - 1;

    xos_memcpy(str, buf, len);
    str[len] = '\0';
    counter = xos_atol(str);

    return size;
}

static void create_counter_kobj(void)
{
    struct kobj_t *root, *test;

    root = kobj_get_root();
    test = kobj_search_directory_with_create(root, "test");

    kobj_add_regular(test, "counter", counter_read, counter_write, &counter);
}
```

### Example 3: Using private data

```c
struct my_data_t {
    int value1;
    int value2;
};

static ssize_t my_data_read(struct kobj_t * kobj, void * buf, size_t size)
{
    struct my_data_t *data = (struct my_data_t *)kobj->priv;
    return xos_snprintf(buf, size, "value1=%d,value2=%d\n", data->value1, data->value2);
}

static void create_kobj_with_priv(void)
{
    struct my_data_t *data;
    struct kobj_t *root, *test;

    data = xos_mem_malloc(sizeof(struct my_data_t));
    if(!data)
        return;

    data->value1 = 10;
    data->value2 = 20;

    root = kobj_get_root();
    test = kobj_search_directory_with_create(root, "test");

    kobj_add_regular(test, "my-data", my_data_read, NULL, data);
}
```

### Example 4: Complete device kobj creation

```c
static ssize_t device_info_read(struct kobj_t * kobj, void * buf, size_t size)
{
    struct device_t *dev = (struct device_t *)kobj->priv;

    return xos_snprintf(buf, size, "name=%s\ndriver=%s\n",
                      dev->name, dev->driver->name);
}

static void create_device_kobj(struct device_t * dev)
{
    struct kobj_t *root, *device_dir, *dev_dir;

    /* Get root node */
    root = kobj_get_root();

    /* Create device directory */
    device_dir = kobj_search_directory_with_create(root, "device");

    /* Create device-specific directory */
    dev_dir = kobj_search_directory_with_create(device_dir, dev->name);

    /* Add device info file */
    kobj_add_regular(dev_dir, "info", device_info_read, NULL, dev);

    /* Add other attribute files... */
}
```

## Practical Applications

### Use in Device Drivers

Each driver module contains a kobj object. When a device is registered, a set of kobj objects are automatically created based on the device name and type.

**Device node path example:**
```
/kobj/device/framebuffer/fb.0/
├── info
├── width
├── height
└── format
```

**Implementation example:**
```c
static struct device_t * my_device_probe(struct driver_t * drv, struct dtnode_t * n)
{
    struct device_t *dev;
    struct kobj_t *root, *device_dir, *dev_dir;

    /* Create device */
    dev = register_my_device(drv, pdat);
    if(!dev)
        return NULL;

    /* Create kobj nodes */
    root = kobj_get_root();
    device_dir = kobj_search_directory_with_create(root, "device");
    dev_dir = kobj_search_directory_with_create(device_dir, dev->name);

    kobj_add_regular(dev_dir, "info", my_device_info_read, NULL, dev);

    return dev;
}
```

### System-level kobj Tree Structure

```
/kobj/
├── device/              /* Device nodes */
│   ├── framebuffer/
│   │   └── fb.0/
│   │       ├── info
│   │       ├── width
│   │       └── height
│   ├── gpio/
│   │   └── gpio.0/
│   │       └── info
│   └── ...
├── class/               /* Class nodes */
│   ├── memory/
│   │   └── meminfo
│   └── ...
└── driver/             /* Driver nodes */
    ├── clk/
    │   └── clk.0/
    │       └── rate
    └── ...
```

### Access via Filesystem

kobj nodes are bound through the sysfs filesystem and can be accessed via filesystem interfaces:

```bash
# View device information
cat /sys/kobj/device/framebuffer/fb.0/info

# View screen width
cat /sys/kobj/device/framebuffer/fb.0/width

# Set clock frequency
echo 24000000 > /sys/kobj/driver/clk/clk.0/rate

# View memory information
cat /sys/kobj/class/memory/meminfo
```

## API Reference

### Node Management

```c
struct kobj_t * kobj_get_root(void);
struct kobj_t * kobj_search(struct kobj_t * parent, const char * name);
struct kobj_t * kobj_search_directory_with_create(struct kobj_t * parent, const char * name);
int kobj_free(struct kobj_t * kobj);
int kobj_add(struct kobj_t * parent, struct kobj_t * kobj);
int kobj_remove(struct kobj_t * parent, struct kobj_t * kobj);
int kobj_remove_self(struct kobj_t * kobj);
```

### Node Creation

```c
struct kobj_t * kobj_alloc_directory(const char * name);
struct kobj_t * kobj_alloc_regular(const char * name, kobj_read_t read, kobj_write_t write, void * priv);
```

### Quick Add

```c
int kobj_add_directory(struct kobj_t * parent, const char * name);
int kobj_add_regular(struct kobj_t * parent, const char * name, kobj_read_t read, kobj_write_t write, void * priv);
```

### Callback Function Types

```c
typedef ssize_t (*kobj_read_t)(struct kobj_t * kobj, void * buf, size_t size);
typedef ssize_t (*kobj_write_t)(struct kobj_t * kobj, void * buf, size_t size);
```

## Best Practices

### 1. Node Naming

- Use lowercase letters and hyphens (kebab-case)
- Names should be descriptive
- Avoid special characters
- Names must be unique under the same parent

### 2. Error Handling

- Always check function return values
- Free allocated memory on add failure
- Use `kobj_remove_self` to clean up entire subtrees

### 3. Thread Safety

- kobj does not use locks, the tree is built during startup (initcall) and is read-mostly at runtime
- Avoid long blocking in callback functions
- Do not call kobj_add/remove inside callback functions (may cause data corruption)

### 4. Memory Management

- Allocate before adding a node, free immediately on failure
- When using private data, ensure its lifecycle matches the node
- When deleting recursively, ensure private data is freed

### 5. Callback Function Design

- Read functions: Return formatted string data
- Write functions: Parse input and update state
- Use private data to pass context
- Return the actual number of bytes read/written

## Summary

kobj is a virtual filesystem in XSTAR for managing kernel objects, with the following features:

1. **Tree structure**: Hierarchical organization for easy management
2. **Type distinction**: Two types, directory and file, with clear purposes
3. **Flexible extension**: Custom functionality through read/write callbacks
4. **No lock required**: kobj tree is built during initcall (single-threaded) and read-only at runtime
5. **Easy access**: Operate through filesystem interfaces

kobj is widely used in device drivers, system monitoring, debugging, and other areas, making it key to understanding XSTAR core components.

For how to use kobj, refer to the implementation in driver modules. For how to access kobj through the filesystem, refer to the sysfs filesystem implementation in the virtual filesystem.
