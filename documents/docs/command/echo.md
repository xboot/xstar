# echo

输出字符串到标准输出。

## 用法

```
echo [option] [string]...
    -n    do not output the trailing newline
```

## 选项

| 选项 | 说明 |
|------|------|
| `-n` | 禁止末尾换行 |

## 说明

- 参数以空格分隔，后跟 `\r\n`
- `-n` 选项禁止末尾换行

## 示例

```bash
# 输出文本
echo Hello World

# 不换行输出
echo -n Hello
```
