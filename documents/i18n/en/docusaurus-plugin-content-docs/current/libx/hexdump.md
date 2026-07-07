# Hex Dump (hexdump)

Outputs memory data in the classic hex dump format, 16 bytes per line, including address, hexadecimal values, and ASCII characters.

## Output Format

```
00000000: 48 65 6c 6c 6f 20 57 6f 72 6c 64 21 00 00 00 00 |Hello World!....|
00000010: 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 |................|
```

Each line consists of three parts:
- **Address** — 8-digit hex address, incremented based on the `base` parameter
- **Hex** — 2 hex digits per byte + space, padded with alignment when fewer than 16 bytes
- **ASCII** — Printable characters (0x20~0x7e) shown as-is, others displayed as `.`

## API

```c
void hexdump(void (*output)(char), unsigned long base, void * buf, int len);
```

- `output` — Character output callback function, called once per character, cannot be NULL
- `base` — Starting address value (for display only, does not affect data reading)
- `buf` — Data buffer to dump
- `len` — Data length (bytes)

## Usage Examples

### Output to shell

```c
static void shell_output(char c)
{
    shell_printf("%c", c);
}

hexdump(shell_output, 0x80000000, data, sizeof(data));
```

### Output to dynamic string

```c
static struct ds_t * g_ds;

static void ds_output(char c)
{
    ds_append_char(g_ds, c);
}

g_ds = ds_alloc();
hexdump(ds_output, 0, buf, len);
/* g_ds contains the complete hex dump text */
ds_free(g_ds);
```

### Debug register dump

```c
hexdump(shell_output, 0x40000000, mmio_base, 64);
/* Displays 64 bytes of register content starting at 0x40000000 */
```
