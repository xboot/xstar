# COBS Codec (cobs)

COBS (Consistent Overhead Byte Stuffing) encodes arbitrary binary data into a byte sequence containing no `0x00` bytes, making it useful for framing messages over byte streams such as UART and TCP: the sender appends a `0x00` after the encoded result as a frame delimiter, and the receiver splits the stream at `0x00` and decodes each frame to recover the original data. The worst-case overhead is 1 byte per 254 bytes of input.

The functions do not allocate memory, add or remove frame delimiters, or calculate CRCs. The header `<libx/cobs.h>` is already included by `<xos/xos.h>`.

## Encoding Principle

The encoder splits the input at `0x00` bytes into runs of non-zero bytes and writes a code byte before each run; the `0x00` separators are implied by the code bytes:

- A code byte ranges from `0x01` to `0xff` and equals the run length plus 1, so a single code byte describes at most 254 non-zero bytes.
- A `0xff` block is emitted only while input remains, so encoded output never contains the redundant form of a `0xff` block followed by a trailing `01` block; the final block's code byte can still be `0xff`, for example an input of exactly 254 non-zero bytes encodes to `ff` plus 254 data bytes, 255 bytes in total.
- The encoded result is at least 1 byte: an empty message encodes to the single byte `01`.
- Neither the code bytes nor the encoded output ever contain `0x00`.

For example, `11 22 00 33` encodes to `03 11 22 02 33`. The first block `03` denotes 2 non-zero bytes followed by a `0x00`; the second block `02` denotes 1 non-zero byte followed by end of input, and the implicit `0x00` of the final block is not part of the data.

## API

```c
#include <libx/cobs.h>

static inline int cobs_encode_bound(unsigned int len);
static inline int cobs_decode_bound(unsigned int len);
int cobs_encode(unsigned char * dst, unsigned int capacity, unsigned char * src, unsigned int len);
int cobs_decode(unsigned char * dst, unsigned int capacity, unsigned char * src, unsigned int len);
```

`len` is the actual input length in bytes, and `capacity` is the number of writable bytes in the destination buffer.

| Return value | Meaning |
| --- | --- |
| `>= 0` | Success; the value is the actual output length. Encoded output is at least 1 byte, so a successful encode returns `>= 1`; decode returns `0` only for an empty message |
| `-1` | Invalid arguments, insufficient output capacity, or malformed encoded input |

### cobs_encode

Encodes `len` bytes from `src` into `dst` and returns the encoded length. It returns `-1` when:

- `dst` is null, or `src` is null while `len > 0`;
- `dst == src` (in-place encoding is unsupported and explicitly rejected);
- `capacity == 0`; even an empty message needs at least 1 byte of capacity;
- `len` is so large that `cobs_encode_bound(len)` overflows to a non-positive value;
- the destination turns out to be too small during encoding.

### cobs_decode

Decodes `len` encoded bytes from `src` into `dst` and returns the decoded length. It returns `-1` when:

- `len == 0` (a zero-length input is not valid encoding);
- `src` is null while `len > 0`, or `dst` is null while `capacity > 0` (`dst` may be null only when `capacity == 0`, in which case only the empty message `01` decodes successfully);
- a code byte is `0x00` (zero bytes must not appear in encoded data);
- a block claims more bytes than the remaining input (truncated frame);
- a `0x00` appears inside block data;
- the destination is too small.

The decoder accepts a final `0xff` block with or without an additional `01` block; both decode to the same result.

### Capacity Bounds

- `cobs_encode_bound(len)` returns the encoded capacity upper bound: `len + len / 254 + 1`.
- `cobs_decode_bound(len)` returns the decoded capacity upper bound: `len - 1` for nonempty input, or `0` for empty input. It does not validate the input format.
- The capacity helpers calculate upper bounds inline. They do not validate input data or check for integer overflow.
- An upper bound is not an exact length. Encoding and decoding succeed whenever the actual result fits; capacity need not reach the upper bound. For example, 254 non-zero bytes encode to 255 bytes, while the upper bound is 256.
- The capacity upper bound excludes the extra frame delimiter; reserve one additional byte when appending a delimiter.

### Overlap and Failure Semantics

- Encoding requires non-overlapping input and output buffers. Decoding supports in-place `dst == src` (the output is never longer than the input); other partial overlaps are unsupported. In-place decoding uses forward byte-by-byte copying.
- An error may leave the destination partially written. Discard the result on failure.

## Examples

Sender side: encode and append the frame delimiter:

```c
unsigned char raw[] = { 0x11, 0x22, 0x00, 0x33 };
unsigned char encoded[16];
int len;

len = cobs_encode(encoded, sizeof(encoded) - 1, raw, sizeof(raw));
if(len < 0)
	return;
encoded[len++] = 0;
```

Encoding produces `03 11 22 02 33`; the caller appends `00` as a frame delimiter, so the first `len` bytes of `encoded` form the complete frame.
The receiver should first split the stream into complete frames at `00`, then pass the exact frame length, excluding the delimiter, to the decoder:

```c
unsigned char frame[] = { 0x03, 0x11, 0x22, 0x02, 0x33 };
int len;

len = cobs_decode(frame, sizeof(frame), frame, sizeof(frame));
if(len < 0)
	return;
```

After in-place decoding, the first `len` bytes of `frame` contain `11 22 00 33`.

When framing at `0x00`, an empty message corresponds to the single-byte frame content `01`, which decodes to length `0`; a frame with no bytes at all between two delimiters is a format error and decoding returns `-1`.

COBS format validation does not replace message integrity checking. Use a CRC at the protocol layer when needed.
