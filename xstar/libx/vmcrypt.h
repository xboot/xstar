#ifndef __XSTAR_LIBX_VMCRYPT_H__
#define __XSTAR_LIBX_VMCRYPT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

/*
 * ┌──────────┬────────────────┬───────────────────────────────────────────────┐
 * │ Opcode   │ Operands       │ Semantics                                     │
 * ├──────────┼────────────────┼───────────────────────────────────────────────┤
 * │ END      │                │ stop execution                                │
 * │ NOP      │                │ no operation                                  │
 * │ MOV      │ rd, rs         │ rd = rs                                       │
 * │ MOVI     │ rd, imm32      │ rd = imm32 (4-byte LE)                        │
 * │ LDR      │ rd, ra         │ regs[rd] = rams[regs[ra]]                     │
 * │ STR      │ rs, ra         │ rams[regs[ra]] = regs[rs]                     │
 * │ ADD      │ r1, r2         │ r1 = r1+r2                                    │
 * │ SUB      │ r1, r2         │ r1 = r1-r2                                    │
 * │ MUL      │ r1, r2         │ r1 = r1*r2                                    │
 * │ DIV      │ r1, r2         │ r1 = r1/r2                                    │
 * │ MOD      │ r1, r2         │ r1 = r1%r2                                    │
 * │ ADDI     │ r, imm32       │ r = r+imm32                                   │
 * │ SUBI     │ r, imm32       │ r = r-imm32                                   │
 * │ MULI     │ r, imm32       │ r = r*imm32                                   │
 * │ DIVI     │ r, imm32       │ r = r/imm32                                   │
 * │ MODI     │ r, imm32       │ r = r%imm32                                   │
 * │ INC      │ r              │ r = r+1                                       │
 * │ DEC      │ r              │ r = r-1                                       │
 * │ NEG      │ r              │ r = -r                                        │
 * │ AND      │ r1, r2         │ r1 = r1 & r2                                  │
 * │ OR       │ r1, r2         │ r1 = r1 | r2                                  │
 * │ XOR      │ r1, r2         │ r1 = r1 ^ r2                                  │
 * │ ANDI     │ r, imm32       │ r = r & imm32                                 │
 * │ ORI      │ r, imm32       │ r = r | imm32                                 │
 * │ XORI     │ r, imm32       │ r = r ^ imm32                                 │
 * │ NOT      │ r              │ r = ~r                                        │
 * │ SHL      │ r, n           │ r = r<<n  (n & 0x1f)                          │
 * │ SHR      │ r, n           │ r = r>>n  (n & 0x1f)                          │
 * │ SHLR     │ r1, r2         │ r1 = r1<<r2 (r2 & 0x1f)                       │
 * │ SHRR     │ r1, r2         │ r1 = r1>>r2 (r2 & 0x1f)                       │
 * │ ROL      │ r, n           │ r = rotl32(r, n)  (n & 0x1f)                  │
 * │ ROR      │ r, n           │ r = rotr32(r, n)  (n & 0x1f)                  │
 * │ ROLR     │ r1, r2         │ r1 = rotl32(r1, r2) (r2 & 0x1f)               │
 * │ RORR     │ r1, r2         │ r1 = rotr32(r1, r2) (r2 & 0x1f)               │
 * │ CMP      │ rd, r1, r2     │ rd = (r1==r2)?1:0                             │
 * │ JMP      │ off32          │ pc += off32 (4-byte LE signed)                │
 * │ JZ       │ r, off32       │ if regs[r]==0: pc += off32                    │
 * │ JNZ      │ r, off32       │ if regs[r]!=0: pc += off32                    │
 * │ IOR      │ rd, ra         │ regs[rd] = buf[regs[ra]]                      │
 * │ IOW      │ rs, ra         │ buf[regs[ra]] = regs[rs] & 0xff               │
 * │ IOL      │ r              │ regs[r] = len                                 │
 * │ TBL      │ rd, ra         │ regs[rd] = prog[regs[ra]]                     │
 * └──────────┴────────────────┴───────────────────────────────────────────────┘
 */
enum {
	VMCRYPT_OP_END   = 0x00,
	VMCRYPT_OP_NOP   = 0x01,
	VMCRYPT_OP_MOV   = 0x02,
	VMCRYPT_OP_MOVI  = 0x03,
	VMCRYPT_OP_LDR   = 0x04,
	VMCRYPT_OP_STR   = 0x05,
	VMCRYPT_OP_ADD   = 0x06,
	VMCRYPT_OP_SUB   = 0x07,
	VMCRYPT_OP_MUL   = 0x08,
	VMCRYPT_OP_DIV   = 0x09,
	VMCRYPT_OP_MOD   = 0x0A,
	VMCRYPT_OP_ADDI  = 0x0B,
	VMCRYPT_OP_SUBI  = 0x0C,
	VMCRYPT_OP_MULI  = 0x0D,
	VMCRYPT_OP_DIVI  = 0x0E,
	VMCRYPT_OP_MODI  = 0x0F,
	VMCRYPT_OP_INC   = 0x10,
	VMCRYPT_OP_DEC   = 0x11,
	VMCRYPT_OP_NEG   = 0x12,
	VMCRYPT_OP_AND   = 0x13,
	VMCRYPT_OP_OR    = 0x14,
	VMCRYPT_OP_XOR   = 0x15,
	VMCRYPT_OP_ANDI  = 0x16,
	VMCRYPT_OP_ORI   = 0x17,
	VMCRYPT_OP_XORI  = 0x18,
	VMCRYPT_OP_NOT   = 0x19,
	VMCRYPT_OP_SHL   = 0x1A,
	VMCRYPT_OP_SHR   = 0x1B,
	VMCRYPT_OP_SHLR  = 0x1C,
	VMCRYPT_OP_SHRR  = 0x1D,
	VMCRYPT_OP_ROL   = 0x1E,
	VMCRYPT_OP_ROR   = 0x1F,
	VMCRYPT_OP_ROLR  = 0x20,
	VMCRYPT_OP_RORR  = 0x21,
	VMCRYPT_OP_CMP   = 0x22,
	VMCRYPT_OP_JMP   = 0x23,
	VMCRYPT_OP_JZ    = 0x24,
	VMCRYPT_OP_JNZ   = 0x25,
	VMCRYPT_OP_IOR   = 0x26,
	VMCRYPT_OP_IOW   = 0x27,
	VMCRYPT_OP_IOL   = 0x28,
	VMCRYPT_OP_TBL   = 0x29,
};

struct vmcrypt_t {
	uint32_t regs[16];
	uint32_t rams[256];
	uint32_t pc;
	uint8_t * prog;
	uint32_t plen;
};

void vmcrypt_init(struct vmcrypt_t * vm, uint8_t * prog, uint32_t plen);
void vmcrypt_reset(struct vmcrypt_t * vm);
void vmcrypt_run(struct vmcrypt_t * vm, uint8_t * buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_VMCRYPT_H__ */
