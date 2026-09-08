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
	if(lock)
		lock->lock = 0;
}

void baremetal_spinlock_exit(struct spinlock_t * lock)
{
}

int baremetal_spinlock_lock(struct spinlock_t * lock)
{
	if(!lock)
		return 0;
	lock->lock = arm32_irq_save();
	__asm__ __volatile__ ("dmb sy" ::: "memory");
	return 1;
}

int baremetal_spinlock_trylock(struct spinlock_t * lock)
{
	if(!lock)
		return 0;
	return baremetal_spinlock_lock(lock);
}

int baremetal_spinlock_unlock(struct spinlock_t * lock)
{
	if(!lock)
		return 0;
	__asm__ __volatile__ ("dmb sy" ::: "memory");
	if(!(lock->lock & (1 << 7)))
		arm32_interrupt_enable();
	lock->lock = 0;
	return 1;
}
