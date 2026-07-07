# dcp

设备间数据拷贝（文件、块设备、内存）。

## 用法

```
dcp <input@offset:size> <output@offset:size>
```

## 说明

- 支持块设备、XFS 文件和裸内存之间的任意组合拷贝
- 格式：`<name>@<offset>:<size>`，省略名称表示裸内存访问
- 偏移和大小支持十进制和十六进制（`0x` 前缀）
- 使用 64KB 缓冲传输
- 显示传输速度和总量

## 示例

```bash
# 从内存拷贝到文件
dcp @0x80000000:0x1000 /tmp/dump.bin@0:0x1000

# 从块设备拷贝到文件
dcp blk.0@0:0x10000 /tmp/backup.bin@0:0x10000
```
