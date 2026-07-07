# cat

显示文件内容。

## 用法

```
cat <file> ...
```

## 说明

- 支持同时显示多个文件
- 以 64KB 块读取并输出到控制台
- 不可打印字符（除 `\r`、`\n`、`\t`、`\f` 外）显示为 `.`
- 对目录或无权限文件会报告错误

## 示例

```bash
# 查看单个文件
cat /kobj/class/memory/meminfo

# 查看多个文件
cat file1.txt file2.txt
```
