# 硬件互斥锁 (hwspinlock)

硬件互斥锁。

## 设备类型

`DEVICE_TYPE_HWSPINLOCK`

## 结构体

```c
struct hwspinlock_t {
    char * name;
    int base, nlock;
    int (*trylock)(struct hwspinlock_t * hsl, int lock);
    void (*lock)(struct hwspinlock_t * hsl, int lock);
    void (*unlock)(struct hwspinlock_t * hsl, int lock);
    void * priv;
};

struct hwspinlock_desc_t {
    struct hwspinlock_t * hsl;
    int offset;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_hwspinlock(lock)` | 按全局锁号查找硬件互斥锁设备 |
| `register_hwspinlock(hsl, drv)` | 注册硬件互斥锁设备 |
| `unregister_hwspinlock(hsl)` | 注销硬件互斥锁设备 |
| `hwspinlock_is_valid(lock)` | 判断全局锁号是否有效 |
| `hwspinlock_trylock(lock)` | 尝试获取指定锁 |
| `hwspinlock_lock(lock)` | 获取指定锁（忙等待） |
| `hwspinlock_unlock(lock)` | 释放指定锁 |
| `hwspinlock_desc_alloc(lock)` | 分配锁描述符 |
| `hwspinlock_desc_free(desc)` | 释放锁描述符 |
| `hwspinlock_desc_trylock/lock/unlock(desc)` | 通过描述符操作锁 |

## 说明

硬件互斥锁接口。每个设备管理一段全局编号的锁空间（`base` ~ `base + nlock - 1`），设备树通过 `hwspinlock-base`、`hwspinlock-count` 属性描述。适用于 AMP 系统中处理器之间以及处理器与硬件模块之间的互斥同步——区别于 `spinlock` 面向 SMP 内核态的互斥。`hwspinlock_desc_t` 描述符缓存了设备指针与锁偏移，适合高频持锁场景，避免每次操作都进行全局锁号查找。
