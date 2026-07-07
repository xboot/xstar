# write

写入字符串到文件。

## 用法

```
write <string> <file>
```

## 说明

- 以写入模式打开文件（创建或截断）
- 写入指定字符串内容
- 空字符串或打开失败会报告错误

## 示例

```bash
write "Hello, World!" /greeting.txt
```
