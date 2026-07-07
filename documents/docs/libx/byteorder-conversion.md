# 字节序转换 (byteorder)

CPU 字节序与小端/大端格式之间的转换接口，用于协议解析、文件格式处理等跨平台场景。

## 背景

不同 CPU 架构的字节序不同（x86 为小端，ARM 可配置），而网络协议用大端（网络字节序），许多文件格式也规定字节序。本模块提供统一的转换接口，消除平台差异。

## 字节序检测

```c
int cpu_is_big_endian(void);
```

运行时检测当前 CPU 是否为大端序。大端返回 1，小端返回 0。

## 转换接口

所有接口为 `static inline`，零额外调用开销。

### CPU ↔ 小端

| 接口 | 说明 |
|------|------|
| `cpu_to_le16(x)` | CPU 字节序 → 16位小端 |
| `le16_to_cpu(x)` | 16位小端 → CPU 字节序 |
| `cpu_to_le32(x)` | CPU 字节序 → 32位小端 |
| `le32_to_cpu(x)` | 32位小端 → CPU 字节序 |
| `cpu_to_le64(x)` | CPU 字节序 → 64位小端 |
| `le64_to_cpu(x)` | 64位小端 → CPU 字节序 |

小端序下 `cpu_to_le` 和 `le_to_cpu` 为恒等操作，大端序下做字节交换。同一方向的转换互为逆操作：`le32_to_cpu(cpu_to_le32(x)) == x`。

### CPU ↔ 大端

| 接口 | 说明 |
|------|------|
| `cpu_to_be16(x)` | CPU 字节序 → 16位大端 |
| `be16_to_cpu(x)` | 16位大端 → CPU 字节序 |
| `cpu_to_be32(x)` | CPU 字节序 → 32位大端 |
| `be32_to_cpu(x)` | 32位大端 → CPU 字节序 |
| `cpu_to_be64(x)` | CPU 字节序 → 64位大端 |
| `be64_to_cpu(x)` | 64位大端 → CPU 字节序 |

大端序下为恒等操作，小端序下做字节交换。

### 通用字节交换

| 接口 | 说明 |
|------|------|
| `__swab16(x)` | 16位字节交换 |
| `__swab32(x)` | 32位字节交换 |
| `__swab64(x)` | 64位字节交换 |
| `__swahw32(x)` | 32位半字交换（高低16位互换） |
| `__swahb32(x)` | 32位半字内字节交换（每16位内部两字节互换） |

## 使用示例

### 解析网络数据包

```c
uint16_t port = be16_to_cpu(*(uint16_t *)packet->port_field);
uint32_t addr = be32_to_cpu(*(uint32_t *)packet->addr_field);
```

### 写入小端文件格式

```c
*(uint32_t *)buf = cpu_to_le32(value);
*(uint16_t *)(buf + 4) = cpu_to_le16(count);
```
