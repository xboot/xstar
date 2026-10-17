# COBS编解码 (cobs)

COBS（Consistent Overhead Byte Stuffing）将任意二进制数据编码为不包含 `0x00` 的字节序列，适合 UART、TCP 等字节流的消息分帧：发送端在编码结果后追加一个 `0x00` 作为帧分隔符，接收端按 `0x00` 切帧后解码即可还原原始数据。最坏开销为每 254 字节输入增加 1 字节。

函数不分配内存，不添加或移除帧分隔符，也不计算 CRC。头文件 `<libx/cobs.h>` 已包含在 `<xos/xos.h>` 中。

## 编码原理

编码器把输入按 `0x00` 切分成若干非零字节段，每段前面写入一个码字节，段内的 `0x00` 由码字节隐含表示：

- 码字节取值 `0x01 ~ 0xff`，等于"本段长度 + 1"，因此单个码字节最多描述 254 个非零字节。
- `0xff` 块只在还有剩余输入时才输出，因此编码结果不会出现 `0xff` 块后紧跟 `01` 结尾块的冗余表示；但末块的码字节本身可以是 `0xff`，例如输入恰好由 254 个非零字节组成时，编码结果为 `ff` 加 254 个数据字节，共 255 字节。
- 编码结果至少 1 个字节：空消息编码为单字节 `01`。
- 码字节不会是 `0x00`，编码结果中也不会出现 `0x00`。

例：`11 22 00 33` 编码为 `03 11 22 02 33`。第一个块 `03` 表示 2 个非零字节后跟一个 `0x00`，第二个块 `02` 表示 1 个非零字节后输入结束，末块的隐含 `0x00` 不属于数据。

## API

```c
#include <libx/cobs.h>

static inline int cobs_encode_bound(unsigned int len);
static inline int cobs_decode_bound(unsigned int len);
int cobs_encode(unsigned char * dst, unsigned int capacity, unsigned char * src, unsigned int len);
int cobs_decode(unsigned char * dst, unsigned int capacity, unsigned char * src, unsigned int len);
```

`len` 是输入的实际字节数，`capacity` 是目标缓冲区可写字节数。

| 返回值 | 含义 |
| --- | --- |
| `>= 0` | 成功，值为实际输出长度。编码结果至少 1 字节，故编码成功返回 `>= 1`；解码仅空消息返回 `0` |
| `-1` | 参数错误、容量不足或编码格式错误 |

### cobs_encode

将 `src` 中 `len` 字节编码到 `dst`，返回编码长度。以下情况返回 `-1`：

- `dst` 为空，或 `len > 0` 时 `src` 为空；
- `dst == src`（原地编码不支持，代码显式检查）；
- `capacity == 0`，编码空消息也需要至少 1 字节容量；
- `len` 过大导致 `cobs_encode_bound(len)` 溢出为非正数；
- 容量不足，写入过程中发现放不下。

### cobs_decode

将 `src` 中 `len` 字节编码数据解码到 `dst`，返回解码长度。以下情况返回 `-1`：

- `len == 0`（零长度不属于合法编码）；
- `len > 0` 时 `src` 为空，或 `capacity > 0` 时 `dst` 为空（`dst` 仅在 `capacity == 0` 时允许为空，此时只能成功解码空消息 `01`）；
- 码字节为 `0x00`（编码结果中不允许出现零）；
- 块长度超出剩余输入（帧被截断）；
- 块内数据中出现 `0x00`；
- 容量不足。

解码器接受末尾 `0xff` 块后有或没有额外 `01` 的合法表示，两者解码结果相同。

### 容量上界

- `cobs_encode_bound(len)` 返回编码容量上界 `len + len / 254 + 1`。
- `cobs_decode_bound(len)` 返回解码容量上界：非空输入为 `len - 1`，空输入为 `0`。此函数不验证输入格式。
- 容量函数内联计算上界，不验证输入数据，也不检查整数溢出。
- 上界不是精确长度。实际输出放得下就成功，无须提供达到上界的容量。例如 254 个非零字节实际编码为 255 字节，上界却是 256。
- 长度上界不包含额外的帧分隔符；需要追加分隔符时，应额外预留一个字节。

### 重叠与失败语义

- 编码的输入输出不得重叠。解码支持 `dst == src` 原地解码（输出始终不长于输入），其他部分重叠不支持。原地解码使用逐字节前向复制。
- 失败时可能已经部分写入目标缓冲区，结果不可使用。

## 使用示例

发送端编码并追加帧分隔符：

```c
unsigned char raw[] = { 0x11, 0x22, 0x00, 0x33 };
unsigned char encoded[16];
int len;

len = cobs_encode(encoded, sizeof(encoded) - 1, raw, sizeof(raw));
if(len < 0)
	return;
encoded[len++] = 0;
```

以上编码得到 `03 11 22 02 33`，外层添加 `00` 作为帧分隔符，`encoded` 前 `len` 字节即完整帧。
接收端应先按 `00` 拆出完整帧，再将不含分隔符的准确长度传入解码器：

```c
unsigned char frame[] = { 0x03, 0x11, 0x22, 0x02, 0x33 };
int len;

len = cobs_decode(frame, sizeof(frame), frame, sizeof(frame));
if(len < 0)
	return;
```

原地解码后，`frame` 的前 `len` 字节为 `11 22 00 33`。

按 `0x00` 分帧时，空消息对应的帧内容是单字节 `01`，解码返回长度 `0`；若两个分隔符之间没有任何字节（零长度帧），属于格式错误，解码返回 `-1`。

COBS 格式检查不能替代消息完整性检查，需要时应在协议层使用 CRC。
