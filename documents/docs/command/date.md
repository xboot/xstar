# date

打印系统日期和时间。

## 用法

```
date [FORMAT]
```

## 说明

- 默认时区为 `Asia/Shanghai`
- 可选参数指定时区字符串（如 `America/New_York`）
- 输出格式：`YYYY-MM-DD HH:MM:SS W`（W=星期，0=星期日）

## 示例

```bash
# 默认时区
date

# 指定时区
date America/New_York
```
