# 复位控制器 (reset)

复位控制器。

## 设备类型

`DEVICE_TYPE_RESETCHIP`

## 结构体

```c
struct resetchip_t {
    char * name;
    int base, nreset;
    int (*assert)(struct resetchip_t * chip, int offset);
    int (*deassert)(struct resetchip_t * chip, int offset);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_resetchip(name)` | 按名称查找复位控制器 |
| `register_resetchip(chip, drv)` | 注册复位控制器 |
| `unregister_resetchip(chip)` | 注销复位控制器 |
| `reset_assert/deassert(chip, offset)` | 断言/取消断言复位 |
| `reset_reset(chip, offset, ms)` | 脉冲复位 |

## 说明

硬件复位线控制器接口。支持单个复位信号的断言和可配置时长的脉冲复位。`resets_t` 聚合类型批量管理多个复位线。
