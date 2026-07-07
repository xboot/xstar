# Base64编解码 (base64)

Base64 编码与解码，遵循 RFC 4648。

## API

```c
int base64_encode_size(int s);
int base64_decode_size(int s);
int base64_encode(const char * in, int len, char * out);
int base64_decode(const char * in, int len, char * out);
```

- `base64_encode_size` — 计算编码后缓冲区大小（含末尾 `\0`），公式为 `((s + 2) / 3) * 4 + 1`
- `base64_decode_size` — 计算解码后缓冲区大小，公式为 `(s / 4) * 3`
- `base64_encode` — 编码，返回输出长度（不含末尾 `\0`）
- `base64_decode` — 解码，返回输出长度

## 使用示例

### 编码

```c
const char * data = "Hello";
int enclen = base64_encode_size(xos_strlen(data));
char * out = xos_mem_malloc(enclen);
base64_encode(data, xos_strlen(data), out);
/* out = "SGVsbG8=" */
xos_mem_free(out);
```

### 解码

```c
const char * b64 = "SGVsbG8=";
int declen = base64_decode_size(xos_strlen(b64));
char * out = xos_mem_malloc(declen);
int len = base64_decode(b64, xos_strlen(b64), out);
/* len = 5, out = "Hello" */
xos_mem_free(out);
```

### 编码→解码往返验证

```c
char in[128] = { ... };
char enc[base64_encode_size(sizeof(in))];
char dec[base64_decode_size(sizeof(enc))];

base64_encode(in, sizeof(in), enc);
int len = base64_decode(enc, sizeof(enc), dec);
/* memcmp(in, dec, sizeof(in)) == 0 */
```
