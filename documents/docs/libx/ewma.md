# 指数加权移动平均 (ewma)

指数加权移动平均滤波器，单极点低通滤波，通过可配置权重将新采样与历史均值混合。

更新公式：`avg = weight * value + (1 - weight) * last`

## 数据结构

```c
struct ewma_filter_t {
    float weight;
    float last;
};
```

## API

```c
struct ewma_filter_t * ewma_alloc(float weight);
void ewma_free(struct ewma_filter_t * filter);
float ewma_update(struct ewma_filter_t * filter, float value);
void ewma_clear(struct ewma_filter_t * filter);
```

- `ewma_alloc` — 分配滤波器，`weight` 范围 0~1，越大越跟随新值
- `ewma_free` — 释放滤波器
- `ewma_update` — 输入新值，返回更新后的均值
- `ewma_clear` — 重置历史状态

## 使用示例

### 传感器数据滤波

```c
struct ewma_filter_t * f = ewma_alloc(0.2f);

while(running)
{
    float sample = read_sensor();
    float filtered = ewma_update(f, sample);
}

ewma_free(f);
```

### 重置滤波器

```c
ewma_clear(f);
```
