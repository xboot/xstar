# vww

VWW (Visual Wake Words) — 轻量级卷积神经网络人体检测库。基于 MobileNet-like 深度可分离卷积架构，INT8 量化，纯 C99 实现，零外部依赖。

## API

```c
#include <vww.h>

int vision_vww(struct vision_t * v);
```

| 参数 | 说明 |
|------|------|
| `v` | 输入图像（支持灰度或 RGB 格式，分辨率须 >= 96x96），内部中心裁剪（保持宽高比，短边对齐 96 像素）后缩放到 96x96。若输入分辨率 < 96x96，需先通过 `vision_resize` 预处理放大 |
| 返回值 | 人体置信度百分比 (0-100) |

## Shell 命令

`vww` 命令用于检测图像中的人体。

```
vww <image>
```

```bash
# 检测图像中的人体
vww person.jpg
# 输出: person: 87%
```

## 网络结构

MobileNet-like 深度可分离卷积网络，共 12 层。

| 层 | 类型 | 输入 | 输出 | 核 | 步长 |
|----|------|------|------|-----|------|
| L0 | CONV2D | 96x96x3 | 48x48x8 | 3x3 | 2x2 |
| L1 | DWCONV2D | 48x48x8 | 48x48x8 | 3x3 | 1x1 |
| L2 | CONV2D | 48x48x8 | 48x48x16 | 1x1 | 1x1 |
| L3 | DWCONV2D | 48x48x16 | 24x24x16 | 3x3 | 2x2 |
| L4 | CONV2D | 24x24x16 | 24x24x32 | 1x1 | 1x1 |
| L5 | DWCONV2D | 24x24x32 | 24x24x32 | 3x3 | 1x1 |
| L6 | CONV2D | 24x24x32 | 24x24x32 | 1x1 | 1x1 |
| L7 | DWCONV2D | 24x24x32 | 12x12x32 | 3x3 | 2x2 |
| L8 | CONV2D | 12x12x32 | 12x12x64 | 1x1 | 1x1 |
| L9 | DWCONV2D | 12x12x64 | 12x12x64 | 3x3 | 1x1 |
| L10 | CONV2D | 12x12x64 | 12x12x64 | 1x1 | 1x1 |
| L11 | DWCONV2D | 12x12x64 | 6x6x64 | 3x3 | 2x2 |

输出经全局平均池化 + 全连接层分类。

## 量化

- 权重: INT8
- 偏置: INT32
- 缩放因子: INT32
- 激活函数: ReLU

所有权重和偏置编译时内嵌在代码中，无需外部模型文件。
