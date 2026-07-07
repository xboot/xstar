# uniqueid

显示系统唯一标识的二维码。

## 用法

```
uniqueid [qrcode invert]
```

## 说明

- 获取系统唯一 ID 并渲染为 ASCII 二维码
- `invert`：非零值为反转色（默认 1），零值为正常色

## 示例

```bash
# 显示二维码
uniqueid

# 正常色显示
uniqueid 0
```
