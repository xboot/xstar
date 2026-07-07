# 均值滤波器 (mean)

滑动窗口均值滤波器，对数据流做移动平均，平滑噪声。O(1) 更新复杂度。

## 原理

维护一个固定长度的环形缓冲区，每次输入新值时减去被覆盖的旧值、加上新值，实现 O(1) 的均值计算，无需遍历整个窗口。

```
窗口 [3, 5, 7]，sum=15，mean=5
输入 9 → 窗口 [5, 7, 9]，sum=21，mean=7
```

## 数据结构

```c
struct mean_filter_t {
    int * buffer;    /* 环形缓冲区 */
    int length;      /* 窗口长度 */
    int index;       /* 当前写入位置 */
    int count;       /* 当前有效样本数 */
    int sum;         /* 窗口内累加和 */
};
```

## API

```c
struct mean_filter_t * mean_alloc(int length);
```

分配滤波器，`length` 为窗口长度。

```c
void mean_free(struct mean_filter_t * filter);
```

释放滤波器。

```c
int mean_update(struct mean_filter_t * filter, int value);
```

输入新值，返回当前窗口均值（整数除法）。窗口未填满时按实际样本数计算。

```c
void mean_clear(struct mean_filter_t * filter);
```

清空缓冲区，重置状态。

## 使用示例

```c
struct mean_filter_t * f = mean_alloc(5);

int v;
while(read_sensor(&v))
{
    int avg = mean_update(f, v);
    printf("raw=%d, avg=%d\n", v, avg);
}

mean_free(f);
```
