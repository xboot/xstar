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
#include <arm32.h>

static inline unsigned long arm32_irq_save(void)
{
	unsigned long old;

	__asm__ __volatile__ (
		"mrs %0, cpsr\n"
		"cpsid i"
		: "=r"(old)
		:
		: "memory");

	return old;
}

void baremetal_spinlock_init(struct spinlock_t * lock)
{
	xatomic_store(&lock->lock, 0);
}

void baremetal_spinlock_exit(struct spinlock_t * lock)
{
}

int baremetal_spinlock_lock(struct spinlock_t * lock)
{
	lock->saved = arm32_irq_save();
	__asm__ __volatile__ ("dmb sy" ::: "memory");
	while(xatomic_xchg(&lock->lock, 1));
	return 1;
}

int baremetal_spinlock_trylock(struct spinlock_t * lock)
{
	unsigned long saved = arm32_irq_save();
	__asm__ __volatile__ ("dmb sy" ::: "memory");
	if(xatomic_xchg(&lock->lock, 1))
	{
		if(!(saved & (1 << 7)))
			arm32_interrupt_enable();
		return 0;
	}
	lock->saved = saved;
	return 1;
}

int baremetal_spinlock_unlock(struct spinlock_t * lock)
{
	__asm__ __volatile__ ("dmb sy" ::: "memory");
	xatomic_store_release(&lock->lock, 0);
	if(!(lock->saved & (1 << 7)))
		arm32_interrupt_enable();
	return 1;
}
