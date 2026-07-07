# 弹簧动画 (spring)

基于物理弹簧模型的动画模块，通过张力和摩擦力参数模拟弹簧运动，产生自然的回弹效果。

## 原理

弹簧运动由两个物理参数控制：

- **张力（tension）** — 弹簧刚度，值越大回弹越快
- **摩擦力（friction）** — 阻尼系数，值越大振荡衰减越快

运动方程：`加速度 = tension × (目标位置 - 当前位置) - friction × 当前速度`

通过欧拉积分以固定步长（1ms）推进仿真，自动处理过冲和振荡。

## 数据结构

```c
struct spring_t {
    float start;       /* 当前位置 */
    float stop;        /* 目标位置 */
    float velocity;    /* 当前速度 */
    float tension;     /* 张力 */
    float friction;    /* 摩擦力 */
    float acc;         /* 时间累计余量 */
};
```

## API

```c
void spring_init(struct spring_t * s, float start, float stop, float velocity, float tension, float friction);
```

初始化弹簧。`start` 为起始位置，`stop` 为目标位置，`velocity` 为初始速度，`tension` 为张力，`friction` 为摩擦力。

```c
int spring_step(struct spring_t * s, float dt);
```

推进 `dt` 秒，内部以 1ms 步长积分。运动未结束返回 1，到达目标（位置和速度均小于 0.005 精度）返回 0。`dt` 上限为 83ms，超过则截断。

```c
float spring_position(struct spring_t * s);
```

获取当前位置。

```c
float spring_velocity(struct spring_t * s);
```

获取当前速度。

## 参数参考

| 效果 | tension | friction |
|------|---------|----------|
| 快速回弹 | 400 | 30 |
| 中等回弹 | 200 | 20 |
| 缓慢回弹 | 100 | 15 |
| 几乎无振荡 | 200 | 40 |
| 明显振荡 | 200 | 10 |

## 使用示例

### 基本弹簧动画

```c
struct spring_t s;
spring_init(&s, 0, 300, 0, 200, 20);

while(spring_step(&s, 0.016))
{
    float pos = spring_position(&s);
    set_position(pos);
}
float final_pos = spring_position(&s);  /* ≈ 300 */
```

### 带初始速度

```c
struct spring_t s;
spring_init(&s, 0, 100, 500, 180, 15);  /* 初速度 500 向目标冲 */
```

### 链式动画

```c
spring_init(&s, current_pos, new_target, 0, 200, 20);
/* 每次目标改变时重新初始化即可 */
```
