# 几何变换 (geometric-transform)

## API

```c
struct vision_t * vision_resize(struct vision_t * v, int width, int height);
```

## 说明

将图像缩放到新尺寸，返回新分配的 Vision 图像。原始图像不变。
