# 硬件自旋锁 (spinlock)

硬件自旋锁。

## 设备类型

`DEVICE_TYPE_SPINLOCK`

## 结构体

```c
struct spinlock_t { int v; };
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_spinlock(name)` | 按名称查找自旋锁设备 |
| `register_spinlock(sl, drv)` | 注册自旋锁设备 |
| `unregister_spinlock(sl)` | 注销自旋锁设备 |
| `spinlock_init(lock)` | 初始化自旋锁 |
| `spinlock_lock(lock)` | 获取自旋锁 |
| `spinlock_trylock(lock)` | 尝试获取自旋锁 |
| `spinlock_unlock(lock)` | 释放自旋锁 |

## 说明

硬件自旋锁接口，用于 SMP 系统中的处理器间同步。区别于原子操作设备——自旋锁专门用于多核场景中的互斥。
