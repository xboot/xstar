# 椭圆曲线签名 (ecdsa256)

椭圆曲线数字签名算法（ECDSA）与椭圆曲线 Diffie-Hellman 密钥交换（ECDH），使用 secp256r1 曲线。

## 常量

```c
#define ECDSA256_BYTES              (32)
#define ECDSA256_PRIVATE_KEY_SIZE   (32)
#define ECDSA256_PUBLIC_KEY_SIZE    (33)
#define ECDSA256_SIGNATURE_SIZE     (64)
#define ECDSA256_SHARED_KEY_SIZE    (32)
```

公钥为压缩格式（33 字节），签名由两个 32 字节大整数拼接而成。

## API

```c
int ecdsa256_keygen(uint8_t * public, uint8_t * private);
```

生成公私钥对。成功返回非零值，失败返回 0。

```c
int ecdsa256_sign(const uint8_t * private, const uint8_t * sha256, uint8_t * signature);
```

使用私钥对 SHA-256 哈希值签名。成功返回非零值，失败返回 0。

```c
int ecdsa256_verify(const uint8_t * public, const uint8_t * sha256, const uint8_t * signature);
```

使用公钥验证签名。验证通过返回非零值，失败返回 0。

```c
int ecdh256_keygen(const uint8_t * public, const uint8_t * private, uint8_t * shared);
```

ECDH 计算共享密钥。成功返回非零值，失败返回 0。

## 使用示例

### 签名与验证

```c
uint8_t pub[ECDSA256_PUBLIC_KEY_SIZE];
uint8_t priv[ECDSA256_PRIVATE_KEY_SIZE];
uint8_t msg[32] = { ... };
uint8_t sign[ECDSA256_SIGNATURE_SIZE];

ecdsa256_keygen(pub, priv);
ecdsa256_sign(priv, msg, sign);

if(ecdsa256_verify(pub, msg, sign))
    /* 签名验证通过 */
```

### ECDH 密钥交换

```c
uint8_t shared[ECDSA256_SHARED_KEY_SIZE];
ecdh256_keygen(remote_public, local_private, shared);
```
