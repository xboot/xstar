# tscal

图形化触摸屏校准。

## 用法

```
tscal <framebuffer> <input>
```

## 说明

- 交互式图形触摸屏校准工具
- 显示 5 个目标点（4 角 + 中心），用户依次点击
- 使用最小二乘法计算 7 参数仿射校准矩阵
- 通过 `input_ioctl(dev, "touchscreen-set-calibration", ...)` 应用校准
- 输出校准数组：`[a0, a1, a2, a3, a4, a5, a6]`
- 校准失败（近奇异矩阵）时自动重启
- 按 Ctrl-C 退出

## 示例

```bash
tscal fb.0 input.0
```
