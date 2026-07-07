# 中值滤波器 (median)

滑动窗口中值滤波器，对数据流取窗口中位数，有效滤除脉冲噪声（毛刺）。O(n) 更新复杂度。

## 原理

维护一个固定长度的环形缓冲区和一个排序索引数组。每次输入新值时，通过插入排序式移动更新索引数组，中值直接取排序后的中间元素。

与均值滤波器相比，中值滤波器能有效剔除突变噪声而不模糊信号边缘：

```
输入:  [100, 102, 999, 98, 101]   ← 999 为毛刺
均值:  280（被毛刺拉高）
中值:  101（毛刺被忽略）
```

## 数据结构

```c
struct median_filter_t {
    int * buffer;    /* 环形缓冲区 */
    int * index;     /* 排序索引数组，index[i] 指向第 i 小的元素位置 */
    int length;      /* 窗口长度 */
    int position;    /* 当前写入位置 */
    int count;       /* 当前有效样本数 */
};
```

## API

```c
struct median_filter_t * median_alloc(int length);
```

分配滤波器，`length` 为窗口长度。

```c
void median_free(struct median_filter_t * filter);
```

释放滤波器。

```c
int median_update(struct median_filter_t * filter, int value);
```

输入新值，返回当前窗口中值。窗口未填满时按实际样本数计算。

```c
void median_clear(struct median_filter_t * filter);
```

清空缓冲区，重置状态。

## 使用示例

### 传感器去毛刺

```c
struct median_filter_t * f = median_alloc(5);

int v;
while(read_sensor(&v))
{
    int filtered = median_update(f, v);
    printf("raw=%d, median=%d\n", v, filtered);
}

median_free(f);
```

### 均值 + 中值级联

```c
struct median_filter_t * mf = median_alloc(5);
struct mean_filter_t * af = mean_alloc(3);

int v;
while(read_sensor(&v))
{
    int med = median_update(mf, v);
    int avg = mean_update(af, med);
    /* avg 为中值滤波后再均值平滑的结果 */
}

median_free(mf);
mean_free(af);
```
