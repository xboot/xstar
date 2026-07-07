# mkdir

创建目录。

## 用法

```
mkdir -v <DIRECTORY> ...
```

## 选项

| 选项 | 说明 |
|------|------|
| `-v` | 详细模式，创建成功时打印 `mkdir '<path>'` |

## 说明

- 支持同时创建多个目录
- 目录创建在 XFS 虚拟文件系统中

## 示例

```bash
# 创建单个目录
mkdir -v /mydir

# 创建多个目录
mkdir -v /dir1 /dir2 /dir3
```
