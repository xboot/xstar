# event

显示所有输入事件。

## 用法

```
event [framebuffer] [input]
```

## 说明

- 分配窗口并将所有输入事件实时导出到控制台
- 支持的事件类型：`KeyDown`、`KeyUp`、`RotaryTurn`、`MouseDown`、`MouseMove`、`MouseUp`、`MouseWheel`、`TouchBegin`、`TouchMove`、`TouchEnd`、`SystemExit`
- 在屏幕上以蓝色线条和红色点标记可视化渲染触摸/鼠标轨迹
- 约 20 FPS 刷新，按 Ctrl-C 退出

## 示例

```bash
event
event fb.0 input.0
```
