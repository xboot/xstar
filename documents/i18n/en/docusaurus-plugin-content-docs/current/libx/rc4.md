# RC4 Cipher (rc4)

RC4 symmetric stream cipher, encrypts/decrypts data in-place. Encryption and decryption use the same operation: applying the same key a second time restores the original plaintext.

## API

```c
void rc4_crypt(uint8_t * key, int kl, uint8_t * dat, int dl);
```

- `key` — Key buffer
- `kl` — Key length in bytes
- `dat` — Data buffer (modified in-place)
- `dl` — Data length in bytes

Data is modified in-place. RC4 is symmetric, so calling `rc4_crypt` with the same key a second time decrypts the data.

## Example

### Encrypt and Decrypt

```c
uint8_t key[] = { 'x', 'b', 'o', 'o', 't' };
uint8_t data[] = { 'h', 'e', 'l', 'l', 'o' };

rc4_crypt(key, sizeof(key), data, sizeof(data));
/* data is now encrypted */

rc4_crypt(key, sizeof(key), data, sizeof(data));
/* data is restored to plaintext */
```

### Encrypt→Decrypt Round-Trip Verification

```c
uint8_t key[] = { 'x', 'b', 'o', 'o', 't' };
uint8_t dat[256];
uint8_t tmp[256];

xos_memcpy(tmp, dat, sizeof(dat));
rc4_crypt(key, 5, tmp, sizeof(tmp));
rc4_crypt(key, 5, tmp, sizeof(tmp));
/* memcmp(dat, tmp, sizeof(dat)) == 0 */
```
