# help

显示命令在线帮助。

## 用法

```
help [command ...]
```

## 说明

- 无参数时列出所有已注册命令及其描述（两列格式）
- 指定命令时显示其名称、描述和详细用法
- 无法识别的命令会提示 "unknown command"

## 示例

```bash
# 列出所有命令
help

# 查看特定命令帮助
help ls
help cat cd
```
