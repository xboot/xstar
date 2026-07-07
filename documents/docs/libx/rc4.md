# RC4加密 (rc4)

RC4 对称流密码，使用密钥对数据原地加密/解密。加密与解密使用相同操作：对密文再次调用即可还原明文。

## API

```c
void rc4_crypt(uint8_t * key, int kl, uint8_t * dat, int dl);
```

- `key` — 密钥缓冲区
- `kl` — 密钥长度（字节）
- `dat` — 数据缓冲区（原地修改）
- `dl` — 数据长度（字节）

数据会被原地修改。RC4 是对称操作，同一密钥加密后再调用一次即可解密。

## 使用示例

### 加密与解密

```c
uint8_t key[] = { 'x', 'b', 'o', 'o', 't' };
uint8_t data[] = { 'h', 'e', 'l', 'l', 'o' };

rc4_crypt(key, sizeof(key), data, sizeof(data));
/* data 已加密 */

rc4_crypt(key, sizeof(key), data, sizeof(data));
/* data 已还原为明文 */
```

### 加密→解密往返验证

```c
uint8_t key[] = { 'x', 'b', 'o', 'o', 't' };
uint8_t dat[256];
uint8_t tmp[256];

xos_memcpy(tmp, dat, sizeof(dat));
rc4_crypt(key, 5, tmp, sizeof(tmp));
rc4_crypt(key, 5, tmp, sizeof(tmp));
/* memcmp(dat, tmp, sizeof(dat)) == 0 */
```
