# ECDSA Signature (ecdsa256)

Elliptic Curve Digital Signature Algorithm and ECDH key exchange using secp256r1 curve.

## Constants

```c
#define ECDSA256_BYTES              (32)
#define ECDSA256_PRIVATE_KEY_SIZE   (32)
#define ECDSA256_PUBLIC_KEY_SIZE    (33)
#define ECDSA256_SIGNATURE_SIZE     (64)
#define ECDSA256_SHARED_KEY_SIZE    (32)
```

Public key is in compressed format (33 bytes). Signature consists of two 32-byte big integers concatenated.

## API

```c
int ecdsa256_keygen(uint8_t * public, uint8_t * private);
```

Generate public/private key pair. Returns non-zero on success, 0 on failure.

```c
int ecdsa256_sign(const uint8_t * private, const uint8_t * sha256, uint8_t * signature);
```

Sign a SHA-256 hash with private key. Returns non-zero on success, 0 on failure.

```c
int ecdsa256_verify(const uint8_t * public, const uint8_t * sha256, const uint8_t * signature);
```

Verify signature with public key. Returns non-zero on valid signature, 0 on failure.

```c
int ecdh256_keygen(const uint8_t * public, const uint8_t * private, uint8_t * shared);
```

ECDH compute shared key. Returns non-zero on success, 0 on failure.

## Example

### Sign and Verify

```c
uint8_t pub[ECDSA256_PUBLIC_KEY_SIZE];
uint8_t priv[ECDSA256_PRIVATE_KEY_SIZE];
uint8_t msg[32] = { ... };
uint8_t sign[ECDSA256_SIGNATURE_SIZE];

ecdsa256_keygen(pub, priv);
ecdsa256_sign(priv, msg, sign);

if(ecdsa256_verify(pub, msg, sign))
    /* signature verified */
```

### ECDH Key Exchange

```c
uint8_t shared[ECDSA256_SHARED_KEY_SIZE];
ecdh256_keygen(remote_public, local_private, shared);
```
