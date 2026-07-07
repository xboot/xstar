# 卡尔曼滤波 (kalman)

单变量线性卡尔曼滤波器，实现预测-更新循环，支持配置状态转移、观测、过程噪声和测量噪声参数。

## 类型

```c
struct kalman_filter_t {
	float a;
	float h;
	float q;
	float r;
	float x;
	float p;
	float k;
};
```

## API

```c
struct kalman_filter_t * kalman_alloc(float a, float h, float q, float r);
void kalman_free(struct kalman_filter_t * filter);
float kalman_update(struct kalman_filter_t * filter, float value);
void kalman_clear(struct kalman_filter_t * filter);
```

- `kalman_alloc` — 分配（a:状态转移, h:观测, q:过程噪声, r:测量噪声）
- `kalman_free` — 释放
- `kalman_update` — 输入测量值，返回更新后的状态估计
- `kalman_clear` — 重置状态

## 使用示例

```c
struct kalman_filter_t * kf = kalman_alloc(1.0f, 1.0f, 0.01f, 0.1f);

float measurements[] = { 10.1f, 9.8f, 10.3f, 9.9f, 10.0f };
for (int i = 0; i < 5; i++)
{
	float estimated = kalman_update(kf, measurements[i]);
	printf("测量: %.1f, 估计: %.2f\n", measurements[i], estimated);
}

kalman_free(kf);
```
