# 电压调节器 (regulator)

电压调节器。

## 设备类型

`DEVICE_TYPE_REGULATOR`

## 结构体

```c
struct regulator_t {
    char * name;
    int count;
    struct regulator_t * (*set_parent)(struct regulator_t * reg, struct regulator_t * parent);
    struct regulator_t * (*get_parent)(struct regulator_t * reg);
    int (*set_enable)(struct regulator_t * reg, int enable);
    int (*get_enable)(struct regulator_t * reg);
    int (*set_voltage)(struct regulator_t * reg, int voltage);
    int (*get_voltage)(struct regulator_t * reg);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_regulator(name)` | 按名称查找调节器 |
| `register_regulator(reg, drv)` | 注册调节器 |
| `unregister_regulator(reg)` | 注销调节器 |
| `regulator_enable/disable(name)` | 使能/禁用 |
| `regulator_set/get_voltage(name, volt)` | 设置/获取电压 |

## 说明

电压调节器接口（如 PMIC、LDO、DC-DC）。控制电源轨的使能/禁用和电压设置，支持级联调节器树和引用计数跟踪。
