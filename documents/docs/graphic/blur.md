# 模糊 (blur)

Box 模糊实现。

## API

```c
void blur(unsigned char * pixel, int width, int height,
          int x, int y, int w, int h, int radius);
```

## 参数

| 参数 | 说明 |
|------|------|
| `pixel` | 像素数据指针 |
| `width, height` | 图像尺寸 |
| `x, y, w, h` | 模糊区域 |
| `radius` | 模糊半径 |

## 说明

对指定矩形区域应用 Box 模糊效果。`radius` 控制模糊强度，较大的值产生更强的模糊。用于毛玻璃等视觉效果。
