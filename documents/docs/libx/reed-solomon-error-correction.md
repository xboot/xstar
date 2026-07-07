# Reed-Solomon 纠错码 (rs)

基于 Reed-Solomon 算法的前向纠错（FEC）模块。在数据中添加校验符号，接收端可自动检测并纠正传输错误，适用于不可靠信道的数据保护。

## 原理

Reed-Solomon 码工作在 GF(2^8) 有限域上，每个符号为 1 字节。编码时为数据生成 `nroots` 个校验符号，解码时可纠正最多 `nroots / 2` 个符号错误（含数据和校验符号）。

```
编码: [数据 len 字节] → [数据 len 字节] + [校验 nroots 字节]
解码: [数据 len 字节] + [校验 nroots 字节] → 纠错后的 [数据 len 字节] + [校验 nroots 字节]
```

## 默认参数

`rsctx_alloc` 使用以下默认参数：

| 参数 | 值 | 说明 |
|------|-----|------|
| 符号大小 | 8 bit | 每符号 1 字节 |
| 域多项式 | 0x11d | GF(2^8) 的本原多项式 |
| 首连续根 | 0 | 生成多项式首根 |
| 本原元素 | 1 | 有限域本原元 |

块长度上限为 255 字节（2^8 - 1），数据长度须满足 `len <= 255 - nroots`。

## 数据结构

```c
struct rsctx_t {
    int mm;                     /* 每符号比特数 */
    int nn;                     /* 每块符号数 = (1 << mm) - 1 */
    unsigned char * alpha_to;   /* 反对数查找表 */
    unsigned char * index_of;   /* 对数查找表 */
    unsigned char * genpoly;    /* 生成多项式 */
    int nroots;                 /* 校验符号数 */
    int fcr;                    /* 首连续根（指数形式） */
    int prim;                   /* 本原元素（指数形式） */
    int iprim;                  /* prim 的逆元 */
};
```

## API

```c
struct rsctx_t * rsctx_alloc(int nroots);
```

分配 RS 编解码上下文。`nroots` 为校验符号数，决定纠错能力（可纠正 `nroots/2` 个符号错误）。返回 NULL 表示失败。

```c
void rsctx_free(struct rsctx_t * ctx);
```

释放上下文及内部查找表。

```c
int rsctx_encode(struct rsctx_t * ctx, unsigned char * data, unsigned int len, unsigned char * parity);
```

编码，为 `data`（长度 `len`）生成 `nroots` 个校验符号写入 `parity`。要求 `len <= 255 - nroots`。成功返回 1，参数无效返回 0。

```c
int rsctx_decode(struct rsctx_t * ctx, unsigned char * data, unsigned int len, unsigned char * parity);
```

解码，对 `data` + `parity` 进行纠错。数据和校验均可能被纠错（原地修改）。无错误时返回 1，成功纠错返回 1，错误不可纠正返回 0。

## 纠错能力

| nroots | 可纠正符号数 | 校验开销 |
|--------|-------------|---------|
| 4 | 2 | 4 字节 |
| 8 | 4 | 8 字节 |
| 16 | 8 | 16 字节 |
| 32 | 16 | 32 字节 |

nroots 越大纠错能力越强，但校验开销也越大，且可用数据长度减少。

## 使用示例

### 编码

```c
int nroots = 16;
struct rsctx_t * ctx = rsctx_alloc(nroots);

unsigned char data[239] = { ... };   /* 255 - 16 = 239 字节数据 */
unsigned char parity[16];

rsctx_encode(ctx, data, 239, parity);

/* 发送 data + parity，共 255 字节 */
rsctx_free(ctx);
```

### 解码纠错

```c
int nroots = 16;
struct rsctx_t * ctx = rsctx_alloc(nroots);

unsigned char data[239];   /* 接收到的数据（可能含错误） */
unsigned char parity[16];  /* 接收到的校验（可能含错误） */

if(rsctx_decode(ctx, data, 239, parity))
    /* 纠错成功，data 和 parity 已恢复正确 */
else
    /* 错误过多，无法纠正 */

rsctx_free(ctx);
```
