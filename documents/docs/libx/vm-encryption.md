# 虚拟机加密 (vmcrypt)

基于自定义虚拟机的数据加解密模块。通过执行一段字节码程序对输入缓冲区进行变换，实现可定制的加解密逻辑。字节码与数据分离的设计使得加密算法可随时替换，而无需重新编译。

## 架构

```
字节码程序(prog) + 输入数据(buf)
        ↓
   vmcrypt 虚拟机执行
        ↓
  16个通用寄存器 + 256个RAM单元
        ↓
  输出变换后的数据(buf)
```

虚拟机在 `vmcrypt_run` 时对 `buf` 原地修改，加解密使用同一套程序，通过不同的字节码实现加密或解密。

## 数据结构

```c
struct vmcrypt_t {
    uint32_t regs[16];     /* 16个32位通用寄存器 (r0~r15) */
    uint32_t rams[256];    /* 256个32位RAM单元 */
    uint32_t pc;           /* 程序计数器 */
    uint8_t * prog;        /* 字节码程序指针 */
    uint32_t plen;         /* 字节码长度 */
};
```

## API

```c
void vmcrypt_init(struct vmcrypt_t * vm, uint8_t * prog, uint32_t plen);
```

初始化虚拟机。`prog` 和 `plen` 为自定义字节码，传 NULL 则使用内置默认加密程序。内部调用 `vmcrypt_reset` 清零寄存器和 RAM。

```c
void vmcrypt_reset(struct vmcrypt_t * vm);
```

重置虚拟机状态（寄存器、RAM、PC 归零），不改变程序指针。

```c
void vmcrypt_run(struct vmcrypt_t * vm, uint8_t * buf, uint32_t len);
```

执行字节码程序，对 `buf`（长度 `len`）进行原地变换。每次运行前自动清零寄存器和 PC，但保留 RAM 内容。

## 指令集

### 控制指令

| 操作码 | 操作数 | 语义 |
|--------|--------|------|
| END (0x00) | | 停止执行 |
| NOP (0x01) | | 空操作 |

### 数据传送

| 操作码 | 操作数 | 语义 |
|--------|--------|------|
| MOV (0x02) | rd, rs | rd = rs |
| MOVI (0x03) | rd, imm32 | rd = imm32（4字节小端） |
| LDR (0x04) | rd, ra | regs[rd] = rams[regs[ra]] |
| STR (0x05) | rs, ra | rams[regs[ra]] = regs[rs] |

### 算术运算

| 操作码 | 操作数 | 语义 |
|--------|--------|------|
| ADD (0x06) | r1, r2 | r1 = r1 + r2 |
| SUB (0x07) | r1, r2 | r1 = r1 - r2 |
| MUL (0x08) | r1, r2 | r1 = r1 * r2 |
| DIV (0x09) | r1, r2 | r1 = r1 / r2（除0停止） |
| MOD (0x0A) | r1, r2 | r1 = r1 % r2（除0停止） |
| ADDI (0x0B) | r, imm32 | r = r + imm32 |
| SUBI (0x0C) | r, imm32 | r = r - imm32 |
| MULI (0x0D) | r, imm32 | r = r * imm32 |
| DIVI (0x0E) | r, imm32 | r = r / imm32（除0停止） |
| MODI (0x0F) | r, imm32 | r = r % imm32（除0停止） |
| INC (0x10) | r | r = r + 1 |
| DEC (0x11) | r | r = r - 1 |
| NEG (0x12) | r | r = -r |

### 位运算

| 操作码 | 操作数 | 语义 |
|--------|--------|------|
| AND (0x13) | r1, r2 | r1 = r1 & r2 |
| OR (0x14) | r1, r2 | r1 = r1 \| r2 |
| XOR (0x15) | r1, r2 | r1 = r1 ^ r2 |
| ANDI (0x16) | r, imm32 | r = r & imm32 |
| ORI (0x17) | r, imm32 | r = r \| imm32 |
| XORI (0x18) | r, imm32 | r = r ^ imm32 |
| NOT (0x19) | r | r = ~r |
| SHL (0x1A) | r, n | r = r &lt;&lt; n（n & 0x1f） |
| SHR (0x1B) | r, n | r = r >> n（n & 0x1f） |
| SHLR (0x1C) | r1, r2 | r1 = r1 &lt;&lt; r2（r2 & 0x1f） |
| SHRR (0x1D) | r1, r2 | r1 = r1 >> r2（r2 & 0x1f） |
| ROL (0x1E) | r, n | r = rotl32(r, n)（n & 0x1f） |
| ROR (0x1F) | r, n | r = rotr32(r, n)（n & 0x1f） |
| ROLR (0x20) | r1, r2 | r1 = rotl32(r1, r2)（r2 & 0x1f） |
| RORR (0x21) | r1, r2 | r1 = rotr32(r1, r2)（r2 & 0x1f） |

### 比较与跳转

| 操作码 | 操作数 | 语义 |
|--------|--------|------|
| CMP (0x22) | rd, r1, r2 | rd = (r1 == r2) ? 1 : 0 |
| JMP (0x23) | off32 | pc += off32（4字节小端有符号） |
| JZ (0x24) | r, off32 | if regs[r]==0: pc += off32 |
| JNZ (0x25) | r, off32 | if regs[r]!=0: pc += off32 |

### I/O 操作

| 操作码 | 操作数 | 语义 |
|--------|--------|------|
| IOR (0x26) | rd, ra | regs[rd] = buf[regs[ra]] |
| IOW (0x27) | rs, ra | buf[regs[ra]] = regs[rs] & 0xff |
| IOL (0x28) | r | regs[r] = len（缓冲区长度） |
| TBL (0x29) | rd, ra | regs[rd] = prog[regs[ra]]（读取程序自身字节） |

## 编码格式

- 操作码：1 字节
- 寄存器编号：1 字节（低4位有效，r0~r15）
- 立即数：4 字节小端序
- 跳转偏移：4 字节小端有符号，基于读取偏移后的 PC 计算

## 内置默认程序

不传入自定义字节码时，使用内置加密程序，逻辑等价于：

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

该算法基于 TEA/XTEA 风格的黄金分割常数，对每个字节用位置相关的密钥做异或变换。由于是异或操作，加密和解密使用同一程序。

## 使用示例

### 使用内置程序加解密

```c
struct vmcrypt_t vm;
vmcrypt_init(&vm, NULL, 0);

uint8_t data[] = "hello world";
vmcrypt_run(&vm, data, sizeof(data));  /* 加密 */
vmcrypt_run(&vm, data, sizeof(data));  /* 解密（恢复原文） */
```

### 使用自定义程序

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

### 保持 RAM 状态的多次运行

```c
vmcrypt_run(&vm, buf1, len1);  /* 第一次运行，RAM 可被程序写入 */
vmcrypt_run(&vm, buf2, len2);  /* 第二次运行，RAM 保留上次结果 */
/* 如需清零 RAM，调用 vmcrypt_reset(&vm) */
```
