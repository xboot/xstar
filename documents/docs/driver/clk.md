# 时钟 (clk)

时钟。

## 设备类型

`DEVICE_TYPE_CLK`

## 结构体

```c
struct clk_t {
    char * name;
    int count;
    struct clk_t * (*set_parent)(struct clk_t * clk, struct clk_t * parent);
    struct clk_t * (*get_parent)(struct clk_t * clk);
    int (*set_enable)(struct clk_t * clk, int enable);
    int (*get_enable)(struct clk_t * clk);
    void (*set_rate)(struct clk_t * clk, uint64_t prate, uint64_t rate);
    uint64_t (*get_rate)(struct clk_t * clk, uint64_t prate);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_clk(name)` | 按名称查找时钟 |
| `register_clk(clk, drv)` | 注册时钟 |
| `unregister_clk(clk)` | 注销时钟 |
| `clk_enable/disable(name)` | 使能/禁用时钟 |
| `clk_set_rate(name, rate)` | 设置时钟频率 |
| `clk_get_rate(name)` | 获取时钟频率 |

## 说明

时钟树管理接口。提供频率配置、父源选择和使能/禁用门控。
