/*
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <xos/xos.h>
#include <libx/vmcrypt.h>

static inline uint32_t vmcrypt_read32(struct vmcrypt_t * vm)
{
	uint32_t v = (uint32_t)vm->prog[vm->pc]
		| ((uint32_t)vm->prog[vm->pc + 1] << 8)
		| ((uint32_t)vm->prog[vm->pc + 2] << 16)
		| ((uint32_t)vm->prog[vm->pc + 3] << 24);
	vm->pc += 4;
	return v;
}

static const uint8_t default_prog[] = {
	VMCRYPT_OP_IOL,  0x05,
	VMCRYPT_OP_MULI, 0x05, 0xB9, 0x79, 0x37, 0x9E,
	VMCRYPT_OP_MOVI, 0x01, 0x78, 0x56, 0x34, 0x12,
	VMCRYPT_OP_XOR,  0x01, 0x05,
	VMCRYPT_OP_MOVI, 0x05, 0x00, 0x00, 0x00, 0x00,
	VMCRYPT_OP_MOV,  0x06, 0x01,
	VMCRYPT_OP_SHL,  0x06, 0x0D,
	VMCRYPT_OP_XOR,  0x01, 0x06,
	VMCRYPT_OP_MOV,  0x06, 0x01,
	VMCRYPT_OP_SHR,  0x06, 0x11,
	VMCRYPT_OP_XOR,  0x01, 0x06,
	VMCRYPT_OP_MOV,  0x06, 0x01,
	VMCRYPT_OP_SHL,  0x06, 0x05,
	VMCRYPT_OP_XOR,  0x01, 0x06,
	VMCRYPT_OP_IOR,  0x00, 0x05,
	VMCRYPT_OP_MOV,  0x06, 0x01,
	VMCRYPT_OP_ANDI, 0x06, 0xFF, 0x00, 0x00, 0x00,
	VMCRYPT_OP_XOR,  0x00, 0x06,
	VMCRYPT_OP_IOW,  0x00, 0x05,
	VMCRYPT_OP_INC,  0x05,
	VMCRYPT_OP_IOL,  0x00,
	VMCRYPT_OP_CMP,  0x06, 0x05, 0x00,
	VMCRYPT_OP_JNZ,  0x06, 0x05, 0x00, 0x00, 0x00,
	VMCRYPT_OP_JMP,  0xC0, 0xFF, 0xFF, 0xFF,
	VMCRYPT_OP_END,
};

void vmcrypt_init(struct vmcrypt_t * vm, uint8_t * prog, uint32_t plen)
{
	if(vm)
	{
		vmcrypt_reset(vm);
		if(prog && (plen > 0))
		{
			vm->prog = prog;
			vm->plen = plen;
		}
		else
		{
			vm->prog = (uint8_t *)default_prog;
			vm->plen = ARRAY_SIZE(default_prog);
		}
	}
}

void vmcrypt_reset(struct vmcrypt_t * vm)
{
	if(vm)
	{
		xos_memset(vm->regs, 0, sizeof(vm->regs));
		xos_memset(vm->rams, 0, sizeof(vm->rams));
		vm->pc = 0;
	}
}

void vmcrypt_run(struct vmcrypt_t * vm, uint8_t * buf, uint32_t len)
{
	int running = 1;

	xos_memset(vm->regs, 0, sizeof(vm->regs));
	vm->pc = 0;

	while(running && (vm->pc < vm->plen))
	{
		switch(vm->prog[vm->pc++])
		{
		case VMCRYPT_OP_END:
			running = 0;
			break;

		case VMCRYPT_OP_NOP:
			break;

		case VMCRYPT_OP_MOV:
		{
			unsigned char rd = vm->prog[vm->pc++] & 0xf;
			unsigned char rs = vm->prog[vm->pc++] & 0xf;
			vm->regs[rd] = vm->regs[rs];
			break;
		}

		case VMCRYPT_OP_MOVI:
		{
			unsigned char rd = vm->prog[vm->pc++] & 0xf;
			uint32_t imm = vmcrypt_read32(vm);
			vm->regs[rd] = imm;
			break;
		}

		case VMCRYPT_OP_LDR:
		{
			unsigned char rd = vm->prog[vm->pc++] & 0xf;
			unsigned char ra = vm->prog[vm->pc++] & 0xf;
			uint32_t addr = vm->regs[ra];
			if(addr < ARRAY_SIZE(vm->rams))
				vm->regs[rd] = vm->rams[addr];
			else
				running = 0;
			break;
		}

		case VMCRYPT_OP_STR:
		{
			unsigned char rs = vm->prog[vm->pc++] & 0xf;
			unsigned char ra = vm->prog[vm->pc++] & 0xf;
			uint32_t addr = vm->regs[ra];
			if(addr < ARRAY_SIZE(vm->rams))
				vm->rams[addr] = vm->regs[rs];
			else
				running = 0;
			break;
		}

		case VMCRYPT_OP_ADD:
		{
			unsigned char r1 = vm->prog[vm->pc++] & 0xf;
			unsigned char r2 = vm->prog[vm->pc++] & 0xf;
			vm->regs[r1] = vm->regs[r1] + vm->regs[r2];
			break;
		}
		case VMCRYPT_OP_SUB:
		{
			unsigned char r1 = vm->prog[vm->pc++] & 0xf;
			unsigned char r2 = vm->prog[vm->pc++] & 0xf;
			vm->regs[r1] = vm->regs[r1] - vm->regs[r2];
			break;
		}
		case VMCRYPT_OP_MUL:
		{
			unsigned char r1 = vm->prog[vm->pc++] & 0xf;
			unsigned char r2 = vm->prog[vm->pc++] & 0xf;
			vm->regs[r1] = vm->regs[r1] * vm->regs[r2];
			break;
		}

		case VMCRYPT_OP_ADDI:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			uint32_t imm = vmcrypt_read32(vm);
			vm->regs[r] = vm->regs[r] + imm;
			break;
		}

		case VMCRYPT_OP_SUBI:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			uint32_t imm = vmcrypt_read32(vm);
			vm->regs[r] = vm->regs[r] - imm;
			break;
		}

		case VMCRYPT_OP_MULI:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			uint32_t imm = vmcrypt_read32(vm);
			vm->regs[r] = vm->regs[r] * imm;
			break;
		}

		case VMCRYPT_OP_DIV:
		{
			unsigned char r1 = vm->prog[vm->pc++] & 0xf;
			unsigned char r2 = vm->prog[vm->pc++] & 0xf;
			uint32_t d = vm->regs[r2];
			if(d != 0)
				vm->regs[r1] = vm->regs[r1] / d;
			else
				running = 0;
			break;
		}

		case VMCRYPT_OP_MOD:
		{
			unsigned char r1 = vm->prog[vm->pc++] & 0xf;
			unsigned char r2 = vm->prog[vm->pc++] & 0xf;
			uint32_t d = vm->regs[r2];
			if(d != 0)
				vm->regs[r1] = vm->regs[r1] % d;
			else
				running = 0;
			break;
		}

		case VMCRYPT_OP_DIVI:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			uint32_t imm = vmcrypt_read32(vm);
			if(imm != 0)
				vm->regs[r] = vm->regs[r] / imm;
			else
				running = 0;
			break;
		}

		case VMCRYPT_OP_MODI:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			uint32_t imm = vmcrypt_read32(vm);
			if(imm != 0)
				vm->regs[r] = vm->regs[r] % imm;
			else
				running = 0;
			break;
		}

		case VMCRYPT_OP_INC:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			vm->regs[r] = vm->regs[r] + 1;
			break;
		}

		case VMCRYPT_OP_DEC:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			vm->regs[r] = vm->regs[r] - 1;
			break;
		}

		case VMCRYPT_OP_NEG:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			vm->regs[r] = 0 - vm->regs[r];
			break;
		}

		case VMCRYPT_OP_AND:
		{
			unsigned char r1 = vm->prog[vm->pc++] & 0xf;
			unsigned char r2 = vm->prog[vm->pc++] & 0xf;
			vm->regs[r1] = vm->regs[r1] & vm->regs[r2];
			break;
		}

		case VMCRYPT_OP_OR:
		{
			unsigned char r1 = vm->prog[vm->pc++] & 0xf;
			unsigned char r2 = vm->prog[vm->pc++] & 0xf;
			vm->regs[r1] = vm->regs[r1] | vm->regs[r2];
			break;
		}

		case VMCRYPT_OP_XOR:
		{
			unsigned char r1 = vm->prog[vm->pc++] & 0xf;
			unsigned char r2 = vm->prog[vm->pc++] & 0xf;
			vm->regs[r1] = vm->regs[r1] ^ vm->regs[r2];
			break;
		}

		case VMCRYPT_OP_ANDI:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			uint32_t imm = vmcrypt_read32(vm);
			vm->regs[r] = vm->regs[r] & imm;
			break;
		}

		case VMCRYPT_OP_ORI:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			uint32_t imm = vmcrypt_read32(vm);
			vm->regs[r] = vm->regs[r] | imm;
			break;
		}

		case VMCRYPT_OP_XORI:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			uint32_t imm = vmcrypt_read32(vm);
			vm->regs[r] = vm->regs[r] ^ imm;
			break;
		}

		case VMCRYPT_OP_NOT:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			vm->regs[r] = ~vm->regs[r];
			break;
		}

		case VMCRYPT_OP_SHL:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			unsigned char n = vm->prog[vm->pc++] & 0x1f;
			vm->regs[r] = vm->regs[r] << n;
			break;
		}

		case VMCRYPT_OP_SHR:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			unsigned char n = vm->prog[vm->pc++] & 0x1f;
			vm->regs[r] = vm->regs[r] >> n;
			break;
		}

		case VMCRYPT_OP_SHLR:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			unsigned char n = vm->regs[vm->prog[vm->pc++] & 0xf] & 0x1f;
			vm->regs[r] = vm->regs[r] << n;
			break;
		}

		case VMCRYPT_OP_SHRR:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			unsigned char n = vm->regs[vm->prog[vm->pc++] & 0xf] & 0x1f;
			vm->regs[r] = vm->regs[r] >> n;
			break;
		}

		case VMCRYPT_OP_ROL:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			unsigned char n = vm->prog[vm->pc++] & 0x1f;
			if(n != 0)
			{
				uint32_t v = vm->regs[r];
				vm->regs[r] = ((v << n) | (v >> (32 - n))) & 0xffffffff;
			}
			break;
		}

		case VMCRYPT_OP_ROR:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			unsigned char n = vm->prog[vm->pc++] & 0x1f;
			if(n != 0)
			{
				uint32_t v = vm->regs[r];
				vm->regs[r] = ((v >> n) | (v << (32 - n))) & 0xffffffff;
			}
			break;
		}

		case VMCRYPT_OP_ROLR:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			unsigned char n = vm->regs[vm->prog[vm->pc++] & 0xf] & 0x1f;
			if(n != 0)
			{
				uint32_t v = vm->regs[r];
				vm->regs[r] = ((v << n) | (v >> (32 - n))) & 0xffffffff;
			}
			break;
		}

		case VMCRYPT_OP_RORR:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			unsigned char n = vm->regs[vm->prog[vm->pc++] & 0xf] & 0x1f;
			if(n != 0)
			{
				uint32_t v = vm->regs[r];
				vm->regs[r] = ((v >> n) | (v << (32 - n))) & 0xffffffff;
			}
			break;
		}

		case VMCRYPT_OP_CMP:
		{
			unsigned char rd = vm->prog[vm->pc++] & 0xf;
			unsigned char r1 = vm->prog[vm->pc++] & 0xf;
			unsigned char r2 = vm->prog[vm->pc++] & 0xf;
			uint32_t v1 = vm->regs[r1];
			uint32_t v2 = vm->regs[r2];
			if(v1 == v2)
				vm->regs[rd] = 1;
			else
				vm->regs[rd] = 0;
			break;
		}

		case VMCRYPT_OP_JMP:
		{
			int32_t off = (int32_t)vmcrypt_read32(vm);
			vm->pc = (uint32_t)((int32_t)vm->pc + off);
			break;
		}

		case VMCRYPT_OP_JZ:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			int32_t off = (int32_t)vmcrypt_read32(vm);
			if(vm->regs[r] == 0)
				vm->pc = (uint32_t)((int32_t)vm->pc + off);
			break;
		}

		case VMCRYPT_OP_JNZ:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			int32_t off = (int32_t)vmcrypt_read32(vm);
			if(vm->regs[r] != 0)
				vm->pc = (uint32_t)((int32_t)vm->pc + off);
			break;
		}

		case VMCRYPT_OP_IOR:
		{
			unsigned char rd = vm->prog[vm->pc++] & 0xf;
			unsigned char ra = vm->prog[vm->pc++] & 0xf;
			uint32_t addr = vm->regs[ra];
			if(buf && (addr < len))
				vm->regs[rd] = buf[addr];
			else
				vm->regs[rd] = 0;
			break;
		}

		case VMCRYPT_OP_IOW:
		{
			unsigned char rs = vm->prog[vm->pc++] & 0xf;
			unsigned char ra = vm->prog[vm->pc++] & 0xf;
			uint32_t addr = vm->regs[ra];
			if(buf && (addr < len))
				buf[addr] = vm->regs[rs] & 0xff;
			break;
		}

		case VMCRYPT_OP_IOL:
		{
			unsigned char r = vm->prog[vm->pc++] & 0xf;
			vm->regs[r] = len;
			break;
		}

		case VMCRYPT_OP_TBL:
		{
			unsigned char rd = vm->prog[vm->pc++] & 0xf;
			unsigned char ra = vm->prog[vm->pc++] & 0xf;
			uint32_t addr = vm->regs[ra];
			if(addr < vm->plen)
				vm->regs[rd] = vm->prog[addr];
			else
				running = 0;
			break;
		}

		default:
			running = 0;
			break;
		}
	}
}
