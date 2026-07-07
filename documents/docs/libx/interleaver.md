# 交织器 (interleaver)

基于伪随机排列的数据交织与解交织模块。通过重新排列字节顺序，将连续的突发错误分散为随机错误，配合纠错码（如 Reed-Solomon）提高抗突发干扰能力。

## 原理

交织将数据按伪随机顺序重新排列，解交织恢复原始顺序。效果示例：

```
原始数据:   [A B C D E F G H]
交织后:     [D A H E B G C F]    ← 按 pattern 重新排列
传输中突发错误:              ↑↑↑
收到数据:   [D A X X B G C F]    ← 连续2字节损坏
解交织后:   [A B C X E X G H]    ← 错误被分散，不再是连续损坏
```

相同的 `size` 和 `seed` 生成相同的交织模式，通信双方只需约定这两个参数即可。

## 数据结构

```c
struct interleaver_t {
    int size;           /* 数据块长度（字节数） */
    int * pattern;      /* 交织模式：pattern[i] 表示输出第 i 个字节取自输入第 pattern[i] 个字节 */
    int * ipattern;     /* 解交织模式：ipattern[i] 表示解交织输出第 i 个字节取自输入第 ipattern[i] 个字节 */
};
```

## API

```c
struct interleaver_t * interleaver_alloc(int size, int seed);
```

分配交织器。`size` 为数据块长度，`seed` 为伪随机生成器种子。内部用 LFSR 伪随机数做 Fisher-Yates 洗牌生成交织模式，同时计算逆模式用于解交织。返回 NULL 表示失败。

```c
void interleaver_free(struct interleaver_t * ctx);
```

释放交织器。

```c
void interleave(struct interleaver_t * ctx, uint8_t * input, uint8_t * output);
```

交织，将 `input` 按 `pattern` 重排写入 `output`。`input` 和 `output` 长度均为 `size` 字节，可为同一缓冲区但结果不正确（需不同缓冲区）。

```c
void deinterleave(struct interleaver_t * ctx, uint8_t * input, uint8_t * output);
```

解交织，将 `input` 按 `ipattern` 恢复原始顺序写入 `output`。同上，`input` 和 `output` 需为不同缓冲区。

## 使用示例

### 与 Reed-Solomon 配合使用

```c
int nroots = 16;
int data_len = 239;
int total = data_len + nroots;

struct rsctx_t * rs = rsctx_alloc(nroots);
struct interleaver_t * il = interleaver_alloc(total, 12345);

unsigned char data[239] = { ... };
unsigned char parity[16];
unsigned char encoded[255];
unsigned char interleaved[255];

rsctx_encode(rs, data, data_len, parity);

memcpy(encoded, data, data_len);
memcpy(encoded + data_len, parity, nroots);
interleave(il, encoded, interleaved);

/* 发送 interleaved ... */

/* 接收端 */
unsigned char received[255];
unsigned char deinterleaved[255];

deinterleave(il, received, deinterleaved);
memcpy(data, deinterleaved, data_len);
memcpy(parity, deinterleaved + data_len, nroots);
rsctx_decode(rs, data, data_len, parity);

interleaver_free(il);
rsctx_free(rs);
```

### 仅交织/解交织

```c
struct interleaver_t * il = interleaver_alloc(64, 42);

uint8_t original[64] = { ... };
uint8_t shuffled[64];
uint8_t restored[64];

interleave(il, original, shuffled);
deinterleave(il, shuffled, restored);

/* restored 与 original 相同 */

interleaver_free(il);
```
