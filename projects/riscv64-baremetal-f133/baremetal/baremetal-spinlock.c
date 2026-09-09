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

#include <baremetal/baremetal.h>
#include <riscv64.h>

void baremetal_spinlock_init(struct spinlock_t * lock)
{
	xatomic_store(&lock->lock, 0);
}

void baremetal_spinlock_exit(struct spinlock_t * lock)
{
}

int baremetal_spinlock_lock(struct spinlock_t * lock)
{
	lock->saved = csr_read_clear(mstatus, MSTATUS_MIE);
	__asm__ __volatile__ ("fence iorw, iorw" ::: "memory");
	while(xatomic_xchg(&lock->lock, 1));
	return 1;
}

int baremetal_spinlock_trylock(struct spinlock_t * lock)
{
	unsigned long saved = csr_read_clear(mstatus, MSTATUS_MIE);
	__asm__ __volatile__ ("fence iorw, iorw" ::: "memory");
	if(xatomic_xchg(&lock->lock, 1))
	{
		if(saved & MSTATUS_MIE)
			csr_set(mstatus, MSTATUS_MIE);
		return 0;
	}
	lock->saved = saved;
	return 1;
}

int baremetal_spinlock_unlock(struct spinlock_t * lock)
{
	__asm__ __volatile__ ("fence iorw, iorw" ::: "memory");
	xatomic_store_release(&lock->lock, 0);
	if(lock->saved & MSTATUS_MIE)
		csr_set(mstatus, MSTATUS_MIE);
	return 1;
}
