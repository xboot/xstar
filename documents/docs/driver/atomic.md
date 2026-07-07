# 硬件原子操作 (atomic)

硬件原子操作。

## 设备类型

`DEVICE_TYPE_ATOMIC`

## 结构体

```c
struct atomic_t { long v; };
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_atomic(name)` | 按名称查找原子操作设备 |
| `register_atomic(atomic, drv)` | 注册原子操作设备 |
| `unregister_atomic(atomic)` | 注销原子操作设备 |
| `atomic_set(atomic, v)` | 设置值 |
| `atomic_get(atomic)` | 获取值 |
| `atomic_add(atomic, v)` | 原子加 |
| `atomic_sub(atomic, v)` | 原子减 |
| `atomic_cas(atomic, o, n)` | 比较并交换 |

## 说明

硬件支持的原子操作。在某些架构上需要特殊的硬件支持或内存屏障。
