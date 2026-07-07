# delay

延时指定时间。

## 用法

```
delay [millisecond]
```

## 说明

- 默认延时 1000ms
- 使用内核 `mdelay()` 忙等待

## 示例

```bash
# 延时默认 1 秒
delay

# 延时 5 秒
delay 5000
```
