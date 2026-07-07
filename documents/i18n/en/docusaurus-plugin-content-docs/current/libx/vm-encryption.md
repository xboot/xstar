# Virtual Machine Encryption (vmcrypt)

A data encryption/decryption module based on a custom virtual machine. It transforms an input buffer by executing a bytecode program, enabling customizable encryption and decryption logic. The separation of bytecode and data allows the encryption algorithm to be replaced at any time without recompilation.

## Architecture

```
Bytecode program (prog) + Input data (buf)
        ↓
   vmcrypt virtual machine execution
        ↓
   16 general-purpose registers + 256 RAM units
        ↓
   Output transformed data (buf)
```

The virtual machine modifies `buf` in-place during `vmcrypt_run`. Encryption and decryption use the same program; different bytecodes implement encryption or decryption.

## Data Structure

```c
struct vmcrypt_t {
    uint32_t regs[16];     /* 16 32-bit general-purpose registers (r0~r15) */
    uint32_t rams[256];    /* 256 32-bit RAM units */
    uint32_t pc;           /* Program counter */
    uint8_t * prog;        /* Bytecode program pointer */
    uint32_t plen;         /* Bytecode length */
};
```

## API

```c
void vmcrypt_init(struct vmcrypt_t * vm, uint8_t * prog, uint32_t plen);
```

Initialize the virtual machine. `prog` and `plen` specify custom bytecode; pass NULL to use the built-in default encryption program. Internally calls `vmcrypt_reset` to zero registers and RAM.

```c
void vmcrypt_reset(struct vmcrypt_t * vm);
```

Reset the virtual machine state (registers, RAM, PC all zeroed), without changing the program pointer.

```c
void vmcrypt_run(struct vmcrypt_t * vm, uint8_t * buf, uint32_t len);
```

Execute the bytecode program, transforming `buf` (length `len`) in-place. Automatically zeroes registers and PC before each run, but retains RAM content.

## Instruction Set

### Control Instructions

| Opcode | Operands | Semantics |
|--------|----------|-----------|
| END (0x00) | | Stop execution |
| NOP (0x01) | | No operation |

### Data Transfer

| Opcode | Operands | Semantics |
|--------|----------|-----------|
| MOV (0x02) | rd, rs | rd = rs |
| MOVI (0x03) | rd, imm32 | rd = imm32 (4-byte little-endian) |
| LDR (0x04) | rd, ra | regs[rd] = rams[regs[ra]] |
| STR (0x05) | rs, ra | rams[regs[ra]] = regs[rs] |

### Arithmetic

| Opcode | Operands | Semantics |
|--------|----------|-----------|
| ADD (0x06) | r1, r2 | r1 = r1 + r2 |
| SUB (0x07) | r1, r2 | r1 = r1 - r2 |
| MUL (0x08) | r1, r2 | r1 = r1 * r2 |
| DIV (0x09) | r1, r2 | r1 = r1 / r2 (stops on division by zero) |
| MOD (0x0A) | r1, r2 | r1 = r1 % r2 (stops on division by zero) |
| ADDI (0x0B) | r, imm32 | r = r + imm32 |
| SUBI (0x0C) | r, imm32 | r = r - imm32 |
| MULI (0x0D) | r, imm32 | r = r * imm32 |
| DIVI (0x0E) | r, imm32 | r = r / imm32 (stops on division by zero) |
| MODI (0x0F) | r, imm32 | r = r % imm32 (stops on division by zero) |
| INC (0x10) | r | r = r + 1 |
| DEC (0x11) | r | r = r - 1 |
| NEG (0x12) | r | r = -r |

### Bitwise Operations

| Opcode | Operands | Semantics |
|--------|----------|-----------|
| AND (0x13) | r1, r2 | r1 = r1 & r2 |
| OR (0x14) | r1, r2 | r1 = r1 \| r2 |
| XOR (0x15) | r1, r2 | r1 = r1 ^ r2 |
| ANDI (0x16) | r, imm32 | r = r & imm32 |
| ORI (0x17) | r, imm32 | r = r \| imm32 |
| XORI (0x18) | r, imm32 | r = r ^ imm32 |
| NOT (0x19) | r | r = ~r |
| SHL (0x1A) | r, n | r = r &lt;&lt; n (n & 0x1f) |
| SHR (0x1B) | r, n | r = r >> n (n & 0x1f) |
| SHLR (0x1C) | r1, r2 | r1 = r1 &lt;&lt; r2 (r2 & 0x1f) |
| SHRR (0x1D) | r1, r2 | r1 = r1 >> r2 (r2 & 0x1f) |
| ROL (0x1E) | r, n | r = rotl32(r, n) (n & 0x1f) |
| ROR (0x1F) | r, n | r = rotr32(r, n) (n & 0x1f) |
| ROLR (0x20) | r1, r2 | r1 = rotl32(r1, r2) (r2 & 0x1f) |
| RORR (0x21) | r1, r2 | r1 = rotr32(r1, r2) (r2 & 0x1f) |

### Comparison and Jumps

| Opcode | Operands | Semantics |
|--------|----------|-----------|
| CMP (0x22) | rd, r1, r2 | rd = (r1 == r2) ? 1 : 0 |
| JMP (0x23) | off32 | pc += off32 (4-byte little-endian signed) |
| JZ (0x24) | r, off32 | if regs[r]==0: pc += off32 |
| JNZ (0x25) | r, off32 | if regs[r]!=0: pc += off32 |

### I/O Operations

| Opcode | Operands | Semantics |
|--------|----------|-----------|
| IOR (0x26) | rd, ra | regs[rd] = buf[regs[ra]] |
| IOW (0x27) | rs, ra | buf[regs[ra]] = regs[rs] & 0xff |
| IOL (0x28) | r | regs[r] = len (buffer length) |
| TBL (0x29) | rd, ra | regs[rd] = prog[regs[ra]] (read byte from program itself) |

## Encoding Format

- Opcode: 1 byte
- Register number: 1 byte (low 4 bits valid, r0~r15)
- Immediate value: 4 bytes little-endian
- Jump offset: 4 bytes little-endian signed, computed relative to PC after reading

## Built-in Default Program

When no custom bytecode is provided, the built-in encryption program is used, with logic equivalent to:

```c
for(uint32_t i = 0; i < len; i++)
{
    uint32_t key = (i * 0x9E3779B9) ^ 0x12345678;
    key ^= (key << 13);
    key ^= (key >> 17);
    key ^= (key << 5);
    buf[i] ^= (key & 0xFF);
}
```

This algorithm uses the TEA/XTEA-style golden ratio constant, applying a position-dependent XOR transformation to each byte. Due to the XOR operation, encryption and decryption use the same program.

## Usage Examples

### Encryption/Decryption with Built-in Program

```c
struct vmcrypt_t vm;
vmcrypt_init(&vm, NULL, 0);

uint8_t data[] = "hello world";
vmcrypt_run(&vm, data, sizeof(data));  /* Encrypt */
vmcrypt_run(&vm, data, sizeof(data));  /* Decrypt (restore original) */
```

### Using a Custom Program

```c
static const uint8_t my_prog[] = {
    VMCRYPT_OP_IOL,  0x05,
    VMCRYPT_OP_IOR,  0x00, 0x05,
    VMCRYPT_OP_XORI, 0x00, 0xAA, 0x00, 0x00, 0x00,
    VMCRYPT_OP_IOW,  0x00, 0x05,
    VMCRYPT_OP_INC,  0x05,
    VMCRYPT_OP_IOL,  0x00,
    VMCRYPT_OP_CMP,  0x06, 0x05, 0x00,
    VMCRYPT_OP_JNZ,  0x06, 0xF0, 0xFF, 0xFF, 0xFF,
    VMCRYPT_OP_END,
};

struct vmcrypt_t vm;
vmcrypt_init(&vm, (uint8_t *)my_prog, sizeof(my_prog));
vmcrypt_run(&vm, data, len);
```

### Multiple Runs Preserving RAM State

```c
vmcrypt_run(&vm, buf1, len1);  /* First run, RAM may be written by program */
vmcrypt_run(&vm, buf2, len2);  /* Second run, RAM retains previous result */
/* Call vmcrypt_reset(&vm) to clear RAM if needed */
```
