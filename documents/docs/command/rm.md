# rm

删除文件或目录。

## 用法

```
rm [-v] <DIRECTORY> ...
```

## 选项

| 选项 | 说明 |
|------|------|
| `-v` | 详细模式，删除成功时打印 `removed '<path>'` |

## 说明

- 支持同时删除多个文件或目录
- 操作在 XFS 虚拟文件系统中

## 示例

```bash
rm -v /tmp/test.txt
rm -v /dir1 /dir2
```
