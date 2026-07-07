# Ring Buffer FIFO (fifo)

Lock-free circular ring buffer FIFO with two API levels: unlocked `__fifo_*` (caller manages synchronization) and locked `fifo_*` (internal mutex). Supports raw byte streams and length-prefixed packets.

## Data Structure

```c
struct fifo_t {
    unsigned char * buffer;
    unsigned int size;
    unsigned int in;
    unsigned int out;
    struct mutex_t lock;
};
```

## API

```c
struct fifo_t * fifo_alloc(unsigned int size);
void fifo_free(struct fifo_t * f);
void fifo_reset(struct fifo_t * f);
int fifo_isempty(struct fifo_t * f);
int fifo_isfull(struct fifo_t * f);
unsigned int fifo_size(struct fifo_t * f);
unsigned int fifo_length(struct fifo_t * f);
unsigned int fifo_available(struct fifo_t * f);
unsigned int fifo_put(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int fifo_get(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int fifo_peek(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int fifo_put_packet(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int fifo_get_packet(struct fifo_t * f, unsigned char * buf, unsigned int len);
```

- `fifo_alloc` — Allocate FIFO with buffer of `size` bytes (rounded to power of 2). Returns NULL on failure
- `fifo_free` — Free FIFO and its buffer
- `fifo_reset` — Reset read/write pointers, effectively emptying the FIFO
- `fifo_isempty` — Return 1 if FIFO is empty
- `fifo_isfull` — Return 1 if FIFO is full
- `fifo_size` — Return total buffer capacity in bytes
- `fifo_length` — Return number of bytes currently stored
- `fifo_available` — Return number of bytes of free space
- `fifo_put` — Write up to `len` bytes from `buf` into FIFO, returns actual bytes written
- `fifo_get` — Read up to `len` bytes from FIFO into `buf`, returns actual bytes read
- `fifo_peek` — Peek up to `len` bytes from FIFO into `buf` without advancing the read pointer, returns actual bytes peeked
- `fifo_put_packet` — Write a length-prefixed packet (4-byte header + data). `len` must be greater than 0. Returns number of data bytes written, or 0 if insufficient space
- `fifo_get_packet` — Read a length-prefixed packet. Returns number of data bytes read, or 0 if no complete packet available. If the caller's buffer is smaller than the packet, the full packet is still consumed from the FIFO, but only `len` bytes are copied and the actual copied length is returned

All `fifo_*` functions use internal mutex. Corresponding `__fifo_*` variants are available without locking for use in interrupt contexts or with external synchronization.

## Example

```c
struct fifo_t * f = fifo_alloc(1024);
if(!f)
    return;

unsigned char tx[] = { 0x01, 0x02, 0x03 };
fifo_put(f, tx, sizeof(tx));

unsigned char rx[3];
unsigned int n = fifo_get(f, rx, sizeof(rx));

fifo_free(f);
```

### Packet Mode

```c
struct fifo_t * f = fifo_alloc(1024);

unsigned char pkt[] = { 0xAA, 0xBB };
fifo_put_packet(f, pkt, sizeof(pkt));

unsigned char buf[64];
unsigned int len = fifo_get_packet(f, buf, sizeof(buf));

fifo_free(f);
```

`fifo_put_packet` automatically prepends a 4-byte length header when writing; `fifo_get_packet` automatically parses it when reading. Zero-length packets are rejected (`len` must be greater than 0). If the consumer buffer is smaller than the packet, the entire packet is still consumed but only `len` bytes are copied, returning the actual copy length. Use `fifo_peek` to obtain the packet length first and prepare an adequate buffer:

```c
unsigned int pkt_len;
if(fifo_peek(f, (unsigned char *)&pkt_len, sizeof(unsigned int)) == sizeof(unsigned int))
{
    unsigned char buf[pkt_len];
    fifo_get_packet(f, buf, pkt_len);
}
```
