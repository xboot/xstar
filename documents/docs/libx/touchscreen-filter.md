# 触摸屏滤波器 (tsfilter)

触摸屏坐标滤波器，对原始触摸数据做中值去毛刺 + 均值平滑 + 仿射校准，输出稳定的屏幕坐标。

## 滤波流程

```
原始坐标 (x, y)
    ↓
中值滤波 (median) — 去除脉冲噪声（毛刺）
    ↓
均值滤波 (mean) — 平滑抖动
    ↓
仿射校准 (cal) — 原始坐标到屏幕坐标的映射
    ↓
输出坐标 (x, y)
```

## 数据结构

```c
struct tsfilter_t {
    struct median_filter_t * mx, * my;   /* X/Y 中值滤波器 */
    struct mean_filter_t * nx, * ny;     /* X/Y 均值滤波器 */
    int cal[7];                          /* 仿射校准参数 */
};
```

## 校准参数

`cal[7]` 定义原始坐标到屏幕坐标的仿射变换：

```
screen_x = (cal[2] + cal[0] * raw_x + cal[1] * raw_y) / cal[6]
screen_y = (cal[5] + cal[3] * raw_x + cal[4] * raw_y) / cal[6]
```

默认值为恒等变换（无旋转、无偏移）：

| 索引 | 默认值 | 含义 |
|------|--------|------|
| cal[0] | 1 | X 缩放系数 |
| cal[1] | 0 | Y→X 旋转系数 |
| cal[2] | 0 | X 偏移 |
| cal[3] | 0 | X→Y 旋转系数 |
| cal[4] | 1 | Y 缩放系数 |
| cal[5] | 0 | Y 偏移 |
| cal[6] | 1 | 整体除数 |

通过五点校准算法可计算出实际参数，消除触摸屏的旋转、缩放和偏移。

## API

```c
struct tsfilter_t * tsfilter_alloc(int ml, int nl);
```

分配滤波器。`ml` 为中值滤波窗口长度，`nl` 为均值滤波窗口长度。

```c
void tsfilter_free(struct tsfilter_t * filter);
```

释放滤波器及内部子滤波器。

```c
void tsfilter_setcal(struct tsfilter_t * filter, int * cal);
```

设置校准参数，`cal` 为 7 元素数组。

```c
void tsfilter_update(struct tsfilter_t * filter, int * x, int * y);
```

输入原始坐标，原地更新为滤波校准后的屏幕坐标。`x`、`y` 为输入输出参数。

```c
void tsfilter_clear(struct tsfilter_t * filter);
```

清空所有子滤波器状态。

## 使用示例

### 基本使用

```c
struct tsfilter_t * f = tsfilter_alloc(5, 4);

int x = raw_x, y = raw_y;
tsfilter_update(f, &x, &y);
/* x, y 为滤波校准后的屏幕坐标 */

tsfilter_free(f);
```

### 带校准参数

```c
struct tsfilter_t * f = tsfilter_alloc(5, 4);

int cal[7] = { 168012, -57, -137136144, -344, 8674, -2840431, 65536 };
tsfilter_setcal(f, cal);

int x = raw_x, y = raw_y;
tsfilter_update(f, &x, &y);

tsfilter_free(f);
```

### 触摸驱动中集成

```c
void ts_event_handler(int raw_x, int raw_y)
{
    int x = raw_x, y = raw_y;
    tsfilter_update(&filter, &x, &y);
    input_report_abs(INPUT_X, x);
    input_report_abs(INPUT_Y, y);
}
```
