# iplay

在屏幕上显示图像。

## 用法

```
iplay [dir|file] [-t=millisecond] [-m=none|contain|cover|fill] [-c=color] [-d=framebuffer]
```

## 选项

| 选项 | 说明 |
|------|------|
| `-t=<ms>` | 每张图片显示时间（默认 0 — 无延时） |
| `-m=<mode>` | 缩放模式：`none`（原始大小）、`contain`（适应）、`cover`（填充裁剪）、`fill`（拉伸） |
| `-c=<color>` | 背景填充色（十六进制，如 `#000000`） |
| `-d=<fb>` | 目标帧缓冲设备名称 |

## 说明

- 显示图像文件；指定目录时按排序顺序显示所有文件
- 支持常见图像格式
- 跳过以 `.` 开头的文件

## 示例

```bash
# 显示单张图片
iplay image.png

# 以幻灯片模式播放目录
iplay /images -t=2000 -m=contain
```
