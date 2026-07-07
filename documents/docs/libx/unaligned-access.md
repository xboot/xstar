# 非对齐访问 (unaligned)

安全地从未对齐内存地址读取或写入多字节数值，避免某些架构上的对齐异常。

## 背景

许多 RISC 架构（如 ARM、MIPS）要求多字节访问必须对齐（如 4 字节 int 必须 4 字节对齐），否则触发异常。而协议数据、文件格式中的字段往往未对齐。本模块通过逐字节拼接的方式安全处理非对齐访问。

## 读取接口

```c
uint16_t get_unaligned_le16(const void * p);
uint16_t get_unaligned_be16(const void * p);
uint32_t get_unaligned_le32(const void * p);
uint32_t get_unaligned_be32(const void * p);
uint64_t get_unaligned_le64(const void * p);
uint64_t get_unaligned_be64(const void * p);
```

从任意地址 `p` 读取指定宽度的数值，按小端（le）或大端（be）解释字节序。`p` 无需对齐。

## 写入接口

```c
void put_unaligned_le16(void * p, uint16_t val);
void put_unaligned_be16(void * p, uint16_t val);
void put_unaligned_le32(void * p, uint32_t val);
void put_unaligned_be32(void * p, uint32_t val);
void put_unaligned_le64(void * p, uint64_t val);
void put_unaligned_be64(void * p, uint64_t val);
```

将数值按指定字节序写入任意地址 `p`，`p` 无需对齐。

## 与 byteorder 的区别

| 模块 | 用途 | 对齐要求 |
|------|------|---------|
| byteorder | CPU 字节序转换 | 地址必须对齐 |
| unaligned | 非对齐地址读写 + 字节序转换 | 地址无需对齐 |

`unaligned` 内部已处理字节序，无需再调用 `byteorder` 转换。适用于直接从字节流中提取字段。

## 使用示例

### 解析协议头部

```c
struct __attribute__((packed)) header {
    uint8_t cmd;
    uint8_t data[3];
};

void parse(const uint8_t * buf)
{
    uint8_t cmd = buf[0];
    uint32_t value = get_unaligned_le32(buf + 1);
    /* value 从 buf+1 开始读取，该地址不是4字节对齐的 */
}
```

### 构造数据包

```c
uint8_t packet[10];
packet[0] = 0xAB;
put_unaligned_be16(packet + 1, 0x1234);
put_unaligned_be32(packet + 3, 0x56789ABC);
/* packet: AB 12 34 56 78 9A BC ... */
```
