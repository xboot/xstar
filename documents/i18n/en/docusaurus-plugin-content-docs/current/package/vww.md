# vww

VWW (Visual Wake Words) — Lightweight convolutional neural network library for human detection. Based on MobileNet-like depthwise separable convolution architecture, INT8 quantized, pure C99 implementation, zero external dependencies.

## API

```c
#include <vww.h>

int vision_vww(struct vision_t * v);
```

| Parameter | Description |
|-----------|-------------|
| `v` | Input image (grayscale or RGB format, resolution must be >= 96x96), internally center-cropped (preserving aspect ratio, shorter side aligned to 96 pixels) then resized to 96x96. If the input resolution is < 96x96, use `vision_resize` to upscale first |
| Return value | Human confidence percentage (0-100) |

## Shell Command

The `vww` command is used to detect humans in an image.

```
vww <image>
```

```bash
# Detect humans in an image
vww person.jpg
# Output: person: 87%
```

## Network Structure

MobileNet-like depthwise separable convolutional network, 12 layers total.

| Layer | Type | Input | Output | Kernel | Stride |
|-------|------|-------|--------|--------|--------|
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

Output goes through global average pooling + fully connected layer for classification.

## Quantization

- Weights: INT8
- Biases: INT32
- Scale factors: INT32
- Activation function: ReLU

All weights and biases are embedded in the code at compile time, no external model file required.
