# 缓动函数 (easing)

基于三次贝塞尔曲线的缓动动画模块，通过定义曲线控制点实现各种加减速效果。

## 贝塞尔曲线

```c
struct bezier_t {
    float ax, bx, cx;   /* X 分量的三次多项式系数 */
    float ay, by, cy;   /* Y 分量的三次多项式系数 */
};
```

标准三次贝塞尔曲线由 4 个控制点定义：P0=(0,0)、P1=(x1,y1)、P2=(x2,y2)、P3=(1,1)。`bezier_init` 将控制点转换为多项式系数，`bezier_calc` 通过牛顿迭代法 + 二分法求值。

```c
void bezier_init(struct bezier_t * b, float x1, float y1, float x2, float y2);
```

初始化贝塞尔曲线，`x1/y1` 和 `x2/y2` 为中间两个控制点，取值范围 [0, 1]。

```c
float bezier_calc(struct bezier_t * b, float t);
```

计算时间 `t`（0~1）对应的曲线值。`t <= 0` 返回 0，`t >= 1` 返回 1。

## 缓动动画

```c
struct easing_t {
    struct bezier_t bezier;   /* 贝塞尔曲线 */
    float start;              /* 起始值 */
    float stop;               /* 终止值 */
    float duration;           /* 持续时间（秒） */
    float acc;                /* 累计时间 */
};
```

### API

```c
void easing_init(struct easing_t * e, float start, float stop, float duration, float x1, float y1, float x2, float y2);
```

初始化缓动动画。`start`/`stop` 为数值范围，`duration` 为总时长，`x1/y1/x2/y2` 为贝塞尔控制点。

```c
float easing_calc(struct easing_t * e, float t);
```

计算时刻 `t`（秒）对应的值。

```c
float easing_step(struct easing_t * e, float dt);
```

推进 `dt` 秒并返回当前值。内部累计时间自动累加。

```c
int easing_finished(struct easing_t * e);
```

动画是否结束，结束返回 1。

## 常用曲线参数

| 效果 | x1 | y1 | x2 | y2 |
|------|-----|-----|-----|-----|
| 线性 | 0 | 0 | 1 | 1 |
| 缓入 | 0.42 | 0 | 1 | 1 |
| 缓出 | 0 | 0 | 0.58 | 1 |
| 缓入缓出 | 0.42 | 0 | 0.58 | 1 |
| 弹性缓出 | 0.68 | -0.55 | 0.27 | 1.55 |

## 使用示例

### 位移动画

```c
struct easing_t e;
easing_init(&e, 0, 300, 0.5, 0.42, 0, 0.58, 1);  /* 缓入缓出，0→300，0.5秒 */

while(!easing_finished(&e))
{
    float pos = easing_step(&e, 0.016);  /* 每帧约16ms */
    set_position(pos);
}
```

### 直接计算指定时刻

```c
struct easing_t e;
easing_init(&e, 0, 100, 1.0, 0, 0, 0.58, 1);
float v = easing_calc(&e, 0.5);  /* 0.5秒时的值 */
```

### 单独使用贝塞尔曲线

```c
struct bezier_t b;
bezier_init(&b, 0.42, 0, 0.58, 1);
float progress = bezier_calc(&b, 0.5);  /* 进度50%时的曲线值 */
```
